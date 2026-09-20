//
// Public fluid simulation parameters.
//

#ifndef CODE_SIMULATION_PARAMS_HPP
#define CODE_SIMULATION_PARAMS_HPP

namespace CodeCuda
{
    namespace FluidSimulation
    {
        struct sim_params
        {
            float density = 1.0f;
            float weight_sor = 1.6f;
            int total_iter_gpu = 650;
            float dt = 1.0f / 120.0f;
            int total_iter_cpu = 60;
            float g = -0.0f;
            float wind_speed = 0.0f;
            float viscosity = 0.1f;
            float smoke_diffuse_coef = 0.0f;
            bool debug = false;
            bool gpu_sim = true;
            float smoke_dissipation = 0.001;
            float velocity_dissipation = 0.01;
        };
    } // namespace FluidSimulation
} // namespace CodeCuda

#endif // CODE_SIMULATION_PARAMS_HPP
