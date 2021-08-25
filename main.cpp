#include <cstdio>
#include <opencv2/opencv.hpp>
#include <thread>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>

struct Color
{
    Color(std::string _display, float _h, float _s, float _v)
    {
        display = _display;
        h = _h * 180;
        s = _s * 255;
        v = _v * 255;
    }
    std::string display;
    float h, s, v;
};

int height, width;
std::queue<std::string> output;
std::queue<cv::Mat> input;
cv::Mat lastMat;
bool stop = false;
std::string chars[]{" ", "░", "▒", "▓", "█"};
const int colorLength = 7;
Color colors[colorLength]{
    Color("\033[31m", 0, 1, 1),
    Color("\033[32m", 0.333, 1, 1),
    Color("\033[33m", 0.167, 1, 1),
    Color("\033[34m", 0.667, 1, 1),
    Color("\033[35m", 0.889, 1, 1),
    Color("\033[36m", 0.5, 1, 1),
    Color("\033[37m", 0, 0, 1)};

void worker()
{
    while (!stop || !input.empty())
    {
        if (!input.empty())
        {
            cv::Mat frame = input.front(), hsvFrame, mat, diff;
            input.pop();
            cv::resize(frame, frame, cv::Size(width, height / 2));
            cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
            cv::cvtColor(frame, mat, cv::COLOR_BGR2GRAY);
            cv::compare(lastMat, mat, diff, cv::CMP_NE);
            std::string imageStr;
            for (int i = 0; i < diff.rows; i++)
            {
                uchar *line = diff.data + i * diff.step;
                int lastCol = -2;
                for (int j = 0; j < diff.cols; j++)
                {
                    if (*line == 255)
                    {
                        cv::Vec3b pixel = hsvFrame.at<cv::Vec3b>(i, j);
                        float minSub = 181;
                        Color color = colors[0];
                        if (lastCol != j - 1)
                            imageStr += "\033[" + std::to_string(i + 1) + ";" + std::to_string(j) + "H";
                        lastCol = j;
                        if (pixel[1] < 40)
                            color = colors[colorLength - 1];
                        else
                        {
                            for (int i = 0; i < colorLength; i++)
                            {
                                float sub = abs(colors[i].h - pixel[0]);
                                if (sub < minSub)
                                {
                                    minSub = sub;
                                    color = colors[i];
                                }
                            }
                        }
                        imageStr += color.display;
                        imageStr += chars[int(pixel[2] * 5 / 256)];
                    }
                    line += 1;
                }
            }
            lastMat = mat;
            output.push(imageStr);
        }
    }
}

int main()
{
    std::string filePath;
    puts("Input video path:");
    std::cin >> filePath;
    puts("Input output width:");
    scanf("%d", &width);
    puts("Input output height:");
    scanf("%d", &height);
    lastMat = cv::Mat::zeros(cv::Size(width, height / 2), CV_8UC1);
    std::thread thread(worker);
    cv::VideoCapture capture;
    cv::Mat frame;
    if (!capture.open(filePath))
    {
        puts("Failed to open video file!");
        return 1;
    }
    std::thread pushThread([&]
                           {
                               while (capture.read(frame))
                               {
                                   input.push(frame);
                               }
                               stop = true;
                           });
    printf("\033[2J");
    int duration = 1000 / capture.get(cv::CAP_PROP_FPS);
    auto last = std::chrono::system_clock::now();
    while (!stop || !output.empty())
    {
        if (!output.empty())
        {
            std::string front = output.front();
            output.pop();
            printf("%s", front.c_str());
            fflush(stdout);
            auto now = std::chrono::system_clock::now();
            int64_t usedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
            std::this_thread::sleep_for(std::chrono::milliseconds(std::max(duration - usedTime, (int64_t)0)));
            last = std::chrono::system_clock::now();
        }
    }
    pushThread.join();
    thread.join();
}