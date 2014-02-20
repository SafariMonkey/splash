/*
 * Copyright (C) 2013 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blobserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobserver.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @window.h
 * The Window class
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "config.h"
#include "coretypes.h"

#include <deque>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "geometry.h"
#include "log.h"
#include "object.h"
#include "texture.h"

namespace Splash {

class Window : public BaseObject
{
    public:
        /**
         * Constructor
         */
        Window(GlWindowPtr w);

        /**
         * Destructor
         */
        ~Window();

        /**
         * No copy constructor, but a move one
         */
        Window(const Window&) = delete;
        Window(Window&& w)
        {
            _isInitialized = w._isInitialized;
            _window = w._window;
            _screenId = w._screenId;
            _screen = w._screen;
            _inTextures = w._inTextures;
            _isLinkedToTexture = w._isLinkedToTexture;
        }

        /**
         * Get the grabbed key
         */
        bool getKey(int key);
        static int getKeys(GLFWwindow*& win, int& key, int& action, int& mods);

        /**
         * Get the grabbed mouse action
         */
        static int getMouseBtn(GLFWwindow* win, int& btn, int& action, int& mods);

        /**
         * Get the mouse position
         */
        static void getMousePos(GLFWwindow* win, int& xpos, int& ypos);

        /**
         * Get the mouse position
         */
        static int getScroll(GLFWwindow* win, double& xoffset, double& yoffset);

        /**
         * Check wether it is initialized
         */
        bool isInitialized() const {return _isInitialized;}

        /**
         * Check wether the given GLFW window is related to this object
         */
        bool isWindow(GLFWwindow* w) const {return (w == _window->get() ? true : false);}

        /**
         * Try to link the given BaseObject to this
         */
        bool linkTo(BaseObjectPtr obj);

        /**
         * Render this window to screen
         */
        bool render();

        /**
         * Set the window to fullscreen
         */
        bool switchFullscreen(int screenId = -1);

        /**
         * Set a new texture to draw
         */
        void setTexture(TexturePtr tex);

        /**
         * Swap the back and front buffers
         */
        void swapBuffers();

    private:
        bool _isInitialized {false};
        GlWindowPtr _window;
        int _screenId {-1};
        bool _fullscreen {false};
        std::vector<Value> _layout {0};

        ObjectPtr _screen;
        glm::mat4x4 _viewProjectionMatrix;
        std::vector<TexturePtr> _inTextures;
        bool _isLinkedToTexture {false}; //< Set to true if the Window is directly connected to a texture, not a Camera or Gui

        static std::mutex _callbackMutex;
        static std::deque<std::pair<GLFWwindow*, std::vector<int>>> _keys; // Input keys queue
        static std::deque<std::pair<GLFWwindow*, std::vector<int>>> _mouseBtn; // Input mouse buttons queue
        static std::pair<GLFWwindow*, std::vector<double>> _mousePos; // Input mouse position
        static std::deque<std::pair<GLFWwindow*, std::vector<double>>> _scroll; // Input mouse scroll queue

        /**
         * Input callbacks
         */
        static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
        static void mouseBtnCallback(GLFWwindow* win, int button, int action, int mods);
        static void mousePosCallback(GLFWwindow* win, double xpos, double ypos);
        static void scrollCallback(GLFWwindow* win, double xoffset, double yoffset);

        /**
         * Register new functors to modify attributes
         */
        void registerAttributes();

        /**
         * Set up the user events callbacks
         */
        void setEventsCallbacks();

        /**
         * Set up the projection surface
         */
        bool setProjectionSurface();
};

typedef std::shared_ptr<Window> WindowPtr;

} // end of namespace

#endif // WINDOW_H
