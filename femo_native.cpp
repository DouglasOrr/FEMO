#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace {
  namespace py = pybind11;
  typedef py::array_t<float, py::array::c_style | py::array::forcecast> float_array;

  float_array simulate_string_cpu(float_array initial,
                                  float propagation,
                                  float damping,
                                  unsigned nsteps) {
    if (initial.ndim() != 2 || initial.shape(0) < 2 || initial.shape(1) < 2) {
      throw std::domain_error("initial array should be 2D, and at least shape (2, 2)");
    }
    const auto nelements = initial.shape(1);
    auto result = float_array(std::vector<long>{nsteps, nelements});

    // Initialize
    const auto state = result.mutable_data(0, 0);
    const auto initial_ptr = initial.data(initial.shape(0) - 2, 0);
    std::copy(initial_ptr, initial_ptr + 2 * nelements, state);

    // Simulate
    for (auto step = 2u; step < nsteps; ++step) {
      // Pin the ends
      state[step * nelements] = 0;
      state[step * nelements + nelements - 1] = 0;
      // Simulate the middle
      for (auto i = 1u; i < nelements - 1; ++i) {
        const auto prev2 = state[(step-2) * nelements + i];
        const auto prev = state[(step-1) * nelements + i];
        const auto left = state[(step-1) * nelements + i - 1];
        const auto right = state[(step-1) * nelements + i + 1];
        state[step * nelements + i] =
          (2-damping)*prev - (1-damping)*prev2 + propagation*(left + right - 2*prev);
      }
    }
    return result;
  }

  __global__
  void run_simulate_string_gpu(float* state, unsigned nsteps, unsigned nelements, float propagation, float damping) {
    extern __shared__ float shared[];
    const auto index = threadIdx.x + 1;

    shared[index] = state[index];
    shared[nelements + index] = state[nelements + index];
    if (threadIdx.x == 0) {
      shared[0] = 0;
      shared[nelements-1] = 0;
      shared[nelements] = 0;
      shared[2*nelements-1] = 0;
      shared[2*nelements] = 0;
      shared[3*nelements-1] = 0;
    }
    for (auto step = 2u; step < nsteps; ++step) {
      __syncthreads();
      const auto prev2 = shared[((step+1) % 3) * nelements + index];
      const auto prev = shared[((step+2) % 3) * nelements + index];
      const auto left = shared[((step+2) % 3) * nelements + index - 1];
      const auto right = shared[((step+2) % 3) * nelements + index + 1];
      const auto next = (2-damping)*prev - (1-damping)*prev2 + propagation*(left + right - 2*prev);
      shared[(step % 3) * nelements + index] = next;
      state[step * nelements + index] = next;
    }
  }

  float_array simulate_string_gpu(float_array initial,
                                  float propagation,
                                  float damping,
                                  unsigned nsteps) {
    if (initial.ndim() != 2 || initial.shape(0) < 2 || initial.shape(1) < 2) {
      throw std::domain_error("initial array should be 2D, and at least shape (2, 2)");
    }
    const auto nelements = initial.shape(1);

    // Allocate & initialize
    float* devState;
    cudaMalloc(&devState, nsteps * nelements * sizeof(float));
    cudaMemcpy(devState, initial.data(initial.shape(0) - 2, 0), 2 * nelements * sizeof(float),
               cudaMemcpyHostToDevice);
    cudaMemset(devState + 2 * nelements, 0, (nsteps-2) * nelements * sizeof(float));

    // Simulate
    const auto nshared_bytes = 3 * nelements * sizeof(float);
    run_simulate_string_gpu<<<1, nelements - 2, nshared_bytes>>>(
      devState, nsteps, nelements, propagation, damping);

    // Copy & free
    auto result = float_array(std::vector<long>{nsteps, nelements});
    cudaDeviceSynchronize();
    cudaMemcpy(result.mutable_data(0, 0), devState, nsteps * nelements * sizeof(float),
               cudaMemcpyDeviceToHost);
    cudaFree(devState);
    return result;
  }

} // namespace (anonymous)

PYBIND11_MODULE(femo_native, m) {
  m.doc() = "Friendly Elastic Melodic Organ native support library";

  m.def("simulate_string_cpu", &simulate_string_cpu, "1D (string) simulation on the CPU",
        py::arg("initial"), py::arg("propagation"), py::arg("damping"), py::arg("nsteps"));
  m.def("simulate_string_gpu", &simulate_string_gpu, "1D (string) simulation on the GPU",
        py::arg("initial"), py::arg("propagation"), py::arg("damping"), py::arg("nsteps"));
}
