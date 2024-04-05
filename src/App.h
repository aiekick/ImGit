#pragma once

class App {
public:
    int run(int argc, char** argv);

public:
    App() = default;   // Prevent construction
    virtual ~App() = default;  // Prevent unwanted destruction

private:
    void m_InitMessaging();

protected:
    App(const App&) = default;  // Prevent construction by copying
    App& operator=(const App&) {
        return *this;
    };  // Prevent assignment
};
