#include <node.h>
#include <SDL.h>
#include <iostream>
#include <string>
#include <cstdlib>


using namespace v8;
using namespace node;

#define JS_METHOD(name) Handle<Value> name(const Arguments &args)
#define JS_GET(name) Handle<Value> name(Local<String> property, const AccessorInfo &info)
#define JS_SET(name)  void name(Local<String> property, Local<Value> value, const AccessorInfo &info)

namespace sdl2 {

    SDL_Event event;

    class Window: public ObjectWrap {
        public:
            static void Init(Handle<Object> target);
           
            Window() {}
            ~Window() {SDL_DestroyWindow(_window);}

            SDL_Window *_window;
            static Persistent<Function> _constructor;
            std::string _title;

            static Window *Ptr(Handle<Value> x);
            static SDL_Window *Val(Handle<Value> x);

        private:
            static JS_METHOD(New);

    };

    Persistent<Function> Window::_constructor;

    Window *Window::Ptr(Handle<Value> x) {
        return ObjectWrap::Unwrap<Window>(x->ToObject());
    }

    SDL_Window *Window::Val(Handle<Value> x) {
        return Ptr(x)->_window;
    }

    void Window::Init(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
        tpl->SetClassName(String::NewSymbol("Window"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        _constructor = Persistent<Function>::New(tpl->GetFunction());
    }

    JS_METHOD(Window::New) {
        // NOTE: Should not be called publicly
        HandleScope scope;

        Window *obj = new Window();
        obj->Wrap(args.This());

        obj->_title = *String::Utf8Value(args[0]->ToString());
        const char *title = obj->_title.c_str();
        int x = args[1]->Int32Value();
        int y = args[2]->Int32Value();
        int w = args[3]->Int32Value();
        int h = args[4]->Int32Value();
        uint32_t flags = args[5]->Uint32Value();

        obj->_window = SDL_CreateWindow(title, x, y, w, h, flags);

        return scope.Close(args.This());
    }

    // =======================================================================

    class GLContext: public ObjectWrap {
        public:
            static void Init(Handle<Object> target);

            GLContext() {}
            ~GLContext() {std::cout << "destructor called\n";}
            SDL_GLContext _context;
            static Persistent<Function> _constructor;
        private:
            static JS_METHOD(New);
    };

    Persistent<Function> GLContext::_constructor;

    void GLContext::Init(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
        tpl->SetClassName(String::NewSymbol("GLContext"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        _constructor = Persistent<Function>::New(tpl->GetFunction());
    }

    JS_METHOD(GLContext::New) {
        HandleScope scope;

        GLContext *obj = new GLContext();
        obj->Wrap(args.This());

        SDL_Window *window = ObjectWrap::Unwrap<Window>(args[0]->ToObject())->_window;
        obj->_context = SDL_GL_CreateContext(window);

        return args.This();
    }
    // ======================================================================


    JS_METHOD(init) {
        HandleScope scope;
        return scope.Close(Integer::New(SDL_Init(args[0]->Uint32Value())));
    }

    JS_METHOD(createWindow) {
        HandleScope scope;

        const unsigned argc = 6;
        Handle<Value> argv[argc] = {args[0], args[1], args[2], args[3], args[4],
            args[5]};
        Local<Object> instance = Window::_constructor->NewInstance(argc, argv);
        Window *obj = ObjectWrap::Unwrap<Window>(instance);
        if (obj->_window) return scope.Close(instance); else return scope.Close(Null());
    }

    JS_METHOD(glCreateContext) {
        HandleScope scope;

        const unsigned argc = 1;
        Handle<Value> argv[argc] = {args[0]};
        Local<Object> instance = GLContext::_constructor->NewInstance(argc, argv);
        GLContext *obj = ObjectWrap::Unwrap<GLContext>(instance);
        if (obj->_context) return scope.Close(instance); else return scope.Close(Null());
    }

    JS_METHOD(delay) {
        HandleScope scope;

        uint32_t ms = args[0]->Uint32Value();
        SDL_Delay(ms);
        return scope.Close(Undefined());
    }

    Local<Object> _ensureobj(Handle<Object> evt, const char *name) {
        HandleScope scope;

        Local<Value> tmp;
        Local<String> _name = String::NewSymbol(name);
        tmp = evt->Get(_name);
        if (!tmp->IsObject()) {
            Local<Object> obj = Object::New();
            evt->Set(_name, obj);
            return scope.Close(obj);
        } else {
            return scope.Close(tmp->ToObject());
        }
    }

    void _setevent(Handle<Object> evt) {
        HandleScope scope;
        
        Local<Object> window;
        Local<Object> key;
        Local<Object> button;
        Local<Object> motion;
        Local<Object> keysym;
        Local<Object> quit;

        evt->Set(String::NewSymbol("type"), Integer::New(event.type));

        switch(event.type) {
            case SDL_WINDOWEVENT: //SDL_WindowEvent window
                window =_ensureobj(evt, "window");
                window->Set(String::NewSymbol("type"), Integer::New(event.window.type));
                window->Set(String::NewSymbol("windowID"), Integer::New(event.window.windowID));
                window->Set(String::NewSymbol("event"), Integer::New(event.window.event));
                window->Set(String::NewSymbol("data1"), Integer::New(event.window.data1));
                window->Set(String::NewSymbol("data2"), Integer::New(event.window.data2));
                break;
            case SDL_KEYDOWN: //SDL_KeyboardEvent key
            case SDL_KEYUP:
                key = _ensureobj(evt, "key");
                key->Set(String::NewSymbol("type"), Integer::New(event.key.type));
                key->Set(String::NewSymbol("windowID"), Integer::New(event.key.windowID));
                // State of the key; SDL_PRESSED or SDL_RELEASED
                key->Set(String::NewSymbol("state"), Integer::New(event.key.state));
                key->Set(String::NewSymbol("repeat"), Integer::New(event.key.repeat));
                keysym = _ensureobj(key, "keysym");
                keysym->Set(String::NewSymbol("scancode"), Integer::New(event.key.keysym.scancode));
                keysym->Set(String::New("sym"), Integer::New(event.key.keysym.sym));
                keysym->Set(String::New("mod"), Integer::New(event.key.keysym.mod));
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                button = _ensureobj(evt, "button");
                button->Set(String::NewSymbol("type"), Integer::New(event.button.type));
                button->Set(String::NewSymbol("timestamp"), Integer::New(event.button.timestamp));
                button->Set(String::NewSymbol("windowID"), Integer::New(event.button.windowID));
                button->Set(String::NewSymbol("button"), Integer::New(event.button.button));
                button->Set(String::NewSymbol("state"), Integer::New(event.button.state));
                button->Set(String::NewSymbol("x"), Integer::New(event.button.x));
                button->Set(String::NewSymbol("y"), Integer::New(event.button.y));
                break;
            case SDL_MOUSEMOTION:
                motion = _ensureobj(evt, "motion");
                motion->Set(String::NewSymbol("type"), Integer::New(event.motion.type));
                motion->Set(String::NewSymbol("timestamp"), Integer::New(event.motion.timestamp));
                motion->Set(String::NewSymbol("windowID"), Integer::New(event.motion.windowID));
                motion->Set(String::NewSymbol("state"), Integer::New(event.motion.state));
                motion->Set(String::NewSymbol("x"), Integer::New(event.motion.x));
                motion->Set(String::NewSymbol("y"), Integer::New(event.motion.y));
                motion->Set(String::NewSymbol("xrel"), Integer::New(event.motion.xrel));
                motion->Set(String::NewSymbol("yrel"), Integer::New(event.motion.yrel));
                break;
            case SDL_QUIT:
                quit = _ensureobj(evt, "quit");
                quit->Set(String::NewSymbol("type"), Integer::New(event.quit.type));
                quit->Set(String::NewSymbol("timestamp"), Integer::New(event.quit.timestamp));
                break;

        }
    }

    JS_METHOD(pollEvent) {
        HandleScope scope;
        int result = SDL_PollEvent(&event);
        if (result == 0) return scope.Close(Integer::New(result));
        _setevent(args[0]->ToObject());
        return scope.Close(Integer::New(result));
    }

    JS_METHOD(waitEvent) {
        HandleScope scope;
        int result = SDL_WaitEvent(&event);
        if (result == 0) return scope.Close(Integer::New(result));
        _setevent(args[0]->ToObject());
        return scope.Close(Integer::New(result));
    }

    JS_METHOD(glSwapWindow) {
        // void glSwapWindow(Window window)
        HandleScope scope;

        SDL_Window *window = ObjectWrap::Unwrap<Window>(args[0]->ToObject())->_window;
        SDL_GL_SwapWindow(window);
        return scope.Close(Undefined());
    }

    JS_METHOD(showWindow) {
        // void SDL_ShowWindow(SDL_Window* window)
        HandleScope scope;

        SDL_Window *window = ObjectWrap::Unwrap<Window>(args[0]->ToObject())->_window;
        SDL_ShowWindow(window);
        return scope.Close(Undefined());
    }

    JS_METHOD(glMakeCurrent) {
        // int SDL_GL_MakeCurrent(SDL_Window*   window,
        //               SDL_GLContext context)
        HandleScope scope;

        SDL_Window *window = ObjectWrap::Unwrap<Window>(args[0]->ToObject())->_window;
        SDL_GLContext context = ObjectWrap::Unwrap<GLContext>(args[0]->ToObject())->_context;
        return scope.Close(Integer::New(SDL_GL_MakeCurrent(window, context)));
    }

    JS_METHOD(getError) {
        // const char* SDL_GetError(void)
        HandleScope scope;

        return String::New(SDL_GetError());
    }

    JS_METHOD(glSetAttribute) {
    //int SDL_GL_SetAttribute(SDL_GLattr attr,
    //                        int        value)

        HandleScope scope;

        SDL_GLattr attr = static_cast<SDL_GLattr>(args[0]->Int32Value());
        int value = args[1]->Int32Value();
        return scope.Close(Integer::New(SDL_GL_SetAttribute(attr, value)));
    }

    JS_METHOD(getWindowID) {
        // Uint32 SDL_GetWindowID(SDL_Window* window)

        HandleScope scope;
        return scope.Close(Integer::New(SDL_GetWindowID(Window::Val(args[0]))));
    }

    JS_METHOD(getKeyName) {
        // const char* SDL_GetKeyName(SDL_Keycode key)
        HandleScope scope;

        SDL_Keycode key = args[0]->Int32Value();

        return scope.Close(String::New(SDL_GetKeyName(key)));
    }

    JS_METHOD(glSetSwapInterval) {
        // int SDL_GL_SetSwapInterval(int interval)
        HandleScope scope;

        int interval = args[0]->Int32Value();
        return scope.Close(Integer::New(SDL_GL_SetSwapInterval(interval)));
    }

}

#define M(name) NODE_SET_METHOD(target, #name, sdl2::name)
#define C(name) target->Set(String::NewSymbol(#name), Integer::New(SDL_ ## name))
#define SDLK(name) target->Set(String::NewSymbol(#name), Integer::New(SDL ## name))

extern void InitConstants(Handle<Object> target);

void InitModule(Handle<Object> target) {
    sdl2::Window::Init(target);
    sdl2::GLContext::Init(target);
    InitConstants(target);

    M(init);
    M(createWindow);
    M(glCreateContext);
    M(delay);
    M(pollEvent);
    M(waitEvent);
    M(glSwapWindow);
    M(showWindow);
    M(glMakeCurrent);
    M(getError);
    M(glSetAttribute);
    M(getWindowID);
    M(getKeyName);
    M(glSetSwapInterval);

    std::atexit(SDL_Quit);
}

NODE_MODULE(pyksdl2, InitModule);
