#include <cxxopts.hpp>
#include <flow/core/Env.hpp>
#include <flow/core/Node.hpp>
#include <flow/core/NodeFactory.hpp>

#include <spdlog/spdlog.h>

#include <atomic>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <span>

struct PreviewNode : public flow::Node
{
    explicit PreviewNode(const flow::UUID& uuid, const std::string& name, std::shared_ptr<flow::Env> env)
        : flow::Node(uuid, flow::TypeName_v<PreviewNode>, name, std::move(env))
    {
        AddInput<std::any>("in", "");
    }

    virtual ~PreviewNode() = default;

    void Compute() override
    {
        std::string result = "";
        if (const auto& in = GetInputData("in"))
        {
            SPDLOG_INFO("Result: {0}", in->ToString());
        }
    }
};

namespace
{
std::atomic_bool should_terminate = false;
void handle_signal(int)
{
    std::cerr << std::endl;
    should_terminate = true;
}
} // namespace

int main(int argc, char** argv)
{
    cxxopts::Options options("Flow");
    options.add_options()("f,flow", "Flow file to open", cxxopts::value<std::string>())(
        "l,log_level", "Logging level [trace = 0, debug = 1, info = 2, warn = 3, err = 4, critical = 5, off = 6]",
        cxxopts::value<int>())("loop", "Flag to run the program on an iterruptable loop.")("h,help", "Print usage");

    cxxopts::ParseResult result;

    try
    {
        result = options.parse(argc, argv);
    }
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cerr << "Caught exception while parsing arguments: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (result.count("help"))
    {
        std::cerr << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if (!result.count("flow"))
    {
        std::cerr << "Flow file required to run" << std::endl;
        return EXIT_FAILURE;
    }

    if (result.count("log_level"))
    {
        spdlog::set_level(static_cast<spdlog::level::level_enum>(result["log_level"].as<int>()));
    }

    std::string filename = result["flow"].as<std::string>();

    json flow_json;
    try
    {
        std::ifstream i;
        i.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        i.open(filename);
        i >> flow_json;
        i.close();
    }
    catch (const std::ios_base::failure& fail)
    {
        SPDLOG_CRITICAL("Failed to load file '{0}': {1}", filename, fail.what());
        return EXIT_FAILURE;
    }

    SPDLOG_INFO("Loaded flow: {0}", filename);

    auto factory = std::make_shared<flow::NodeFactory>();
    factory->RegisterNodeClass<PreviewNode>("console", "Output");

    SPDLOG_TRACE("Loading modules...");
    auto env = flow::Env::Create(factory);
    env->LoadModules(std::filesystem::current_path() / "modules");
    SPDLOG_INFO("Loaded modules");

    flow::Graph graph("main", env);
    graph.OnError.Bind("Log", [](const std::exception& e) { SPDLOG_ERROR("Caught graph exception: {0}", e.what()); });
    flow_json.get_to(graph);

    SPDLOG_TRACE("Starting nodes...");
    graph.Visit([](const auto& node) { node->Start(); });
    SPDLOG_INFO("Started nodes");

    SPDLOG_INFO("Running flow...");
    graph.Run();
    env->Wait();

    std::signal(SIGINT, handle_signal);

    if (result.count("loop"))
    {
        while (!should_terminate)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            graph.Run();
            env->Wait();
        }
    }

    SPDLOG_INFO("Done");

    SPDLOG_TRACE("Stopping nodes...");
    graph.Visit([](const auto& node) { node->Stop(); });
    SPDLOG_INFO("Stopped nodes");

    SPDLOG_INFO("Flow complete, exiting...");

    return EXIT_SUCCESS;
}
