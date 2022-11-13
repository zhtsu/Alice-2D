#include "Application.hpp"
#include "Alice/Log/Log.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Alice/Core/Timestep.hpp"
#include "Alice/Renderer/Renderer.hpp"

namespace Alice
{

Application* Application::s_instance = nullptr;

Application::Application()
{
    ALICE_ASSERT(!s_instance, "Application already exists!");
    s_instance = this;
    
    Alice::Log::Init();

    m_window = std::unique_ptr<Window>(Window::Create());
    m_window->SetEventCallback(ALICE_BIND_EVENT_FN(Application::OnEvent));

    Renderer::Init();    

    m_imgui_layer = std::make_unique<ImGuiLayer>();
    PushOverlay(m_imgui_layer.get());
}

Application::~Application()
{

}

void Application::OnEvent(Event &event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowCloseEvent>(ALICE_BIND_EVENT_FN(Application::OnWindowClose));

    // ALICE_TRACE("{}", event);

    for (auto it = m_layer_stack.end(); it != m_layer_stack.begin(); )
    {
        (*--it)->OnEvent(event);
        if (event.m_handled)
        {
            break;
        }
    }
}

void Application::Run()
{
    while(m_running)
    {
        float time = (float)glfwGetTime();
        Timestep timestep = time - m_last_frame_time;
        m_last_frame_time = time;

        for (Layer* layer : m_layer_stack)
        {
            layer->OnUpdate(timestep);
        }

        m_imgui_layer->Begin();
        for (Layer* layer : m_layer_stack)
        {
            layer->OnImGuiRender();
        }
        m_imgui_layer->End();

        // ALICE_TRACE("{}, {}", Input::GetMouseX(), Input::GetMouseY());

        m_window->OnUpdate();
    }
}

void Application::PushLayer(Layer* layer)
{
    m_layer_stack.PushLayer(layer);
    layer->OnAttach();
}

void Application::PushOverlay(Layer* layer)
{
    m_layer_stack.PushOverlay(layer);
    layer->OnAttach();
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
    m_running = false;
    
    return true;
}

} // namespace Alice
