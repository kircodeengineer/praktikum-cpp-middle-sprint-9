#include "mandelbrot_sender.hpp"
#include "sfml_events_handler.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <queue>
#include <stdexec/execution.hpp>
#include <thread>

namespace ex = stdexec;

TEST(ViewportCalculationsTest, CorrectWidthAndHeight) {
    ViewPort standard_view{-3.0, 1.0, -2.0, 2.0};
    EXPECT_DOUBLE_EQ(standard_view.width(), 4.0);
    EXPECT_DOUBLE_EQ(standard_view.height(), 4.0);

    ViewPort zero_width{5.0, 5.0, -1.0, 1.0};
    EXPECT_DOUBLE_EQ(zero_width.width(), 0.0);
    EXPECT_DOUBLE_EQ(zero_width.height(), 2.0);

    ViewPort zero_height{-2.0, 2.0, 0.5, 0.5};
    EXPECT_DOUBLE_EQ(zero_height.width(), 4.0);
    EXPECT_DOUBLE_EQ(zero_height.height(), 0.0);

    ViewPort negative_coords{-5.0, -3.0, -4.0, -1.0};
    EXPECT_DOUBLE_EQ(negative_coords.width(), 2.0);
    EXPECT_DOUBLE_EQ(negative_coords.height(), 3.0);
}

TEST(PerformanceCounterTest, AverageComputationWorks) {
    AvrTimeCounter performance_meter{};

    performance_meter.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    performance_meter.End();

    performance_meter.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    performance_meter.End();

    performance_meter.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    performance_meter.End();

    EXPECT_EQ(performance_meter.Count(), 3);
    EXPECT_GT(performance_meter.GetAvr(), 19.0);
    EXPECT_LT(performance_meter.GetAvr(), 23.0);

    performance_meter.Reset();
    EXPECT_EQ(performance_meter.Count(), 0);
    EXPECT_DOUBLE_EQ(performance_meter.GetAvr(), 0.0);
}

TEST(FractalRendererTest, BufferGenerationCorrect) {
    const int test_width{16};
    const int test_height{8};
    RenderSettings render_config{
        .width = test_width, .height = test_height, .max_iterations = 50, .escape_radius = 2.5};
    ViewPort fractal_viewport{};

    auto fractal_generator{mandelbrot::MakeComputeSender(render_config, fractal_viewport)};
    auto render_result{ex::sync_wait(fractal_generator)};

    ASSERT_TRUE(render_result.has_value());

    auto frame_buffer{*render_result};
    ASSERT_NE(std::get<0>(frame_buffer), nullptr);

    EXPECT_EQ(std::get<0>(frame_buffer)->rgba.size(), test_width * test_height * 4);

    bool has_non_zero{};
    for (uint8_t pixel_value : std::get<0>(frame_buffer)->rgba) {
        if (pixel_value != 0) {
            has_non_zero = true;
            break;
        }
    }
    EXPECT_TRUE(has_non_zero);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
