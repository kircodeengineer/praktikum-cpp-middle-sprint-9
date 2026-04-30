#pragma once

#include "mandelbrot_fractal_utils.hpp"
#include "types_sfml.hpp"

#include <print>
#include <ranges>
#include <stdexec/execution.hpp>

using namespace std::chrono_literals;
namespace ex = stdexec;

namespace mandelbrot {

static auto MakeComputeSender(RenderSettings settings, ViewPort viewport) {
    static AvrTimeCounter time_counter;

    return ex::just() | ex::then([=]() mutable -> FrameBuffer * {
               time_counter.Start();

               static FrameBuffer fb{FrameBuffer::Make(settings.width, settings.height)};

               const auto w{settings.width};
               const auto h{settings.height};

               for (auto y : std::views::iota(0u, h)) {
                   for (auto x : std::views::iota(0u, w)) {
                       const auto c{Pixel2DToComplex(x, y, viewport, w, h)};

                       const auto iterations{
                           CalculateIterationsForPoint(c, settings.max_iterations, settings.escape_radius)};

                       const auto color{IterationsToColor(iterations, settings.max_iterations)};

                       const auto idx{(static_cast<std::size_t>(y) * w + x) * 4u};
                       fb.rgba[idx + 0] = color.r;
                       fb.rgba[idx + 1] = color.g;
                       fb.rgba[idx + 2] = color.b;
                       fb.rgba[idx + 3] = 255;
                   }
               }

               return &fb;
           }) |
           ex::then([](FrameBuffer *fb) {
               time_counter.End();
               if (time_counter.Count() % 10 == 0) {
                   std::println("\nAverage compute time: {} ms over {} frames", time_counter.GetAvr(),
                                time_counter.Count());
               }
               return fb;
           });
}

}  // namespace mandelbrot
