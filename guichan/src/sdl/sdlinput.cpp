/*      _______   __   __   __   ______   __   __   _______   __   __                 
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\                
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /                 
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /                  
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /                   
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /                    
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/                      
 *
 * Copyright (c) 2004, 2005 darkbits                        Js_./
 * Per Larsson a.k.a finalman                          _RqZ{a<^_aa
 * Olof Naess�n a.k.a jansem/yakslem                _asww7!uY`>  )\a//
 *                                                 _Qhm`] _f "'c  1!5m
 * Visit: http://guichan.darkbits.org             )Qk<P ` _: :+' .'  "{[
 *                                               .)j(] .d_/ '-(  P .   S
 * License: (BSD)                                <Td/Z <fP"5(\"??"\a.  .L
 * Redistribution and use in source and          _dV>ws?a-?'      ._/L  #'
 * binary forms, with or without                 )4d[#7r, .   '     )d`)[
 * modification, are permitted provided         _Q-5'5W..j/?'   -?!\)cam'
 * that the following conditions are met:       j<<WP+k/);.        _W=j f
 * 1. Redistributions of source code must       .$%w\/]Q  . ."'  .  mj$
 *    retain the above copyright notice,        ]E.pYY(Q]>.   a     J@\
 *    this list of conditions and the           j(]1u<sE"L,. .   ./^ ]{a
 *    following disclaimer.                     4'_uomm\.  )L);-4     (3=
 * 2. Redistributions in binary form must        )_]X{Z('a_"a7'<a"a,  ]"[
 *    reproduce the above copyright notice,       #}<]m7`Za??4,P-"'7. ).m
 *    this list of conditions and the            ]d2e)Q(<Q(  ?94   b-  LQ/
 *    following disclaimer in the                <B!</]C)d_, '(<' .f. =C+m
 *    documentation and/or other materials      .Z!=J ]e []('-4f _ ) -.)m]'
 *    provided with the distribution.          .w[5]' _[ /.)_-"+?   _/ <W"
 * 3. Neither the name of Guichan nor the      :$we` _! + _/ .        j?
 *    names of its contributors may be used     =3)= _f  (_yQmWW$#(    "
 *    to endorse or promote products derived     -   W,  sQQQQmZQ#Wwa]..
 *    from this software without specific        (js, \[QQW$QWW#?!V"".
 *    prior written permission.                    ]y:.<\..          .
 *                                                 -]n w/ '         [.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT       )/ )/           !
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY         <  (; sac    ,    '
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING,               ]^ .-  %
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF            c <   r
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR            aga<  <La
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE          5%  )P'-3L
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR        _bQf` y`..)a
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          ,J?4P'.P"_(\?d'.,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES               _Pa,)!f/<[]/  ?"
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT      _2-..:. .r+_,.. .
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     ?a.<%"'  " -'.a_ _,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION)                     ^
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * For comments regarding functions please see the header file. 
 */

#include "guichan/sdl/sdlinput.hpp"
#include "guichan/exception.hpp"

extern uint32_t SDL_CUSTOM_KEY_UP;

// Ugh... This entire file is so redundant with sdl.cpp - but I don't feel like refactoring the entire guichan mess...
// So... more mess!!
static char ControlIsDown = 0;

static bool isTextInput(int key) {
	return key >= 32 && key < 128 && !ControlIsDown;
}

namespace gcn
{
    SDLInput::SDLInput()
    {
        mMouseInWindow = true;
		mMouseDown = false;
		mIsRepeating = false;
    }
    
    bool SDLInput::isKeyQueueEmpty()
    {
        return mKeyInputQueue.empty();
    }
  
    KeyInput SDLInput::dequeueKeyInput()
    {
        KeyInput keyInput;
    
        if (mKeyInputQueue.empty())
        {
            throw GCN_EXCEPTION("The queue is empty.");
        }
    
        keyInput = mKeyInputQueue.front();
        mKeyInputQueue.pop();

        return keyInput;    
    }

    bool SDLInput::isMouseQueueEmpty()
    {
        return mMouseInputQueue.empty();
    }

    MouseInput SDLInput::dequeueMouseInput()
    {
        MouseInput mouseInput;
    
        if (mMouseInputQueue.empty())
        {
            throw GCN_EXCEPTION("The queue is empty.");
        }
    
        mouseInput = mMouseInputQueue.front();
        mMouseInputQueue.pop();

        return mouseInput;    
    }
    
	void SDLInput::processKeyRepeat()
	{
		KeyInput keyInput;

		if (mIsRepeating) {
			keyInput.setKey(mLastKey);
			keyInput.setType(KeyInput::PRESS);
			mKeyInputQueue.push(keyInput);
		}
	}

    void SDLInput::pushInput(SDL_Event event)
    {
        KeyInput keyInput;
        MouseInput mouseInput;
    
        switch (event.type)
        {
          case SDL_TEXTINPUT:
              {
                  char* text = event.text.text;
                  if (isTextInput((uint8_t)text[0])) {
                      mLastKey = text[0];
                      mIsRepeating = true;
                      keyInput.setKey(mLastKey);
                      keyInput.setType(KeyInput::PRESS);
                      mKeyInputQueue.push(keyInput);
                  }
              }
              break;

          case SDL_KEYDOWN:
              if (!isTextInput(event.key.keysym.sym)) {
                  switch (event.key.keysym.sym) {
                      case SDLK_LCTRL:
                        ControlIsDown |= 0b01;
                        break;
                      case SDLK_RCTRL:
                        ControlIsDown |= 0b10;
                  }
                  mLastKey = convertKeyCharacter(event.key.keysym);
                  mIsRepeating = true;
                  keyInput.setKey(mLastKey);
                  keyInput.setType(KeyInput::PRESS);         
                  mKeyInputQueue.push(keyInput);
              }
              break;

          case SDL_KEYUP:
              if (!isTextInput(event.key.keysym.sym)) {
                  switch (event.key.keysym.sym) {
                      case SDLK_LCTRL:
                        ControlIsDown &= 0b10;
                        break;
                      case SDLK_RCTRL:
                        ControlIsDown &= 0b01;
                  }
                  mIsRepeating = false;
                  keyInput.setKey(convertKeyCharacter(event.key.keysym));
                  keyInput.setType(KeyInput::RELEASE);
                  mKeyInputQueue.push(keyInput);
              }
              break;

          case SDL_MOUSEBUTTONDOWN:
              mMouseDown = true;
              mouseInput.x = event.button.x;
              mouseInput.y = event.button.y;
              mouseInput.setButton(convertMouseButton(event.button.button));
              mouseInput.setType(MouseInput::PRESS);
              mouseInput.setTimeStamp(SDL_GetTicks());
              mMouseInputQueue.push(mouseInput);
              break;

          case SDL_MOUSEBUTTONUP:
              mMouseDown = false;        
              mouseInput.x = event.button.x;
              mouseInput.y = event.button.y;
              mouseInput.setButton(convertMouseButton(event.button.button));
              mouseInput.setType(MouseInput::RELEASE);
              mouseInput.setTimeStamp(SDL_GetTicks());
              mMouseInputQueue.push(mouseInput);
              break;
        
          case SDL_MOUSEWHEEL:
              if (event.wheel.y != 0) {
                  SDL_GetMouseState(&mouseInput.x, &mouseInput.y);
                  if (event.wheel.y > 0)
                    mouseInput.setType(MouseInput::WHEEL_UP);
                  else
                    mouseInput.setType(MouseInput::WHEEL_DOWN);
                  mouseInput.setButton(MouseInput::EMPTY);
                  mouseInput.setTimeStamp(event.wheel.timestamp);
                  mMouseInputQueue.push(mouseInput);
              }
              break;

          case SDL_MOUSEMOTION:
              mouseInput.x = event.button.x;
              mouseInput.y = event.button.y;
              mouseInput.setButton(MouseInput::EMPTY);
              mouseInput.setType(MouseInput::MOTION);
              mouseInput.setTimeStamp(SDL_GetTicks());
              mMouseInputQueue.push(mouseInput);
              break;
        
          case SDL_WINDOWEVENT:
              /* 
               * This occurs when the mouse leaves the window and the Gui-chan
               * application loses its mousefocus.
               */
              switch (event.window.event)
              {
                  case SDL_WINDOWEVENT_LEAVE:
                      {
                          mMouseInWindow = false;

                          if (!mMouseDown)
                          {
                              mouseInput.x = -1;
                              mouseInput.y = -1;
                              mouseInput.setButton(MouseInput::EMPTY);
                              mouseInput.setType(MouseInput::MOTION);
                              mMouseInputQueue.push(mouseInput);
                          }
                      }
                      break;

                  case SDL_WINDOWEVENT_ENTER:
                      mMouseInWindow = true;
                      break;
              }
              break;

            default:
              if (event.type == SDL_CUSTOM_KEY_UP) {
                  mIsRepeating = false;
                  keyInput.setKey(static_cast<char>(event.user.code));
                  keyInput.setType(KeyInput::RELEASE);
                  mKeyInputQueue.push(keyInput);
              }
              break;
        
        } // end switch
    }
  
    int SDLInput::convertMouseButton(int button)
    {
        switch (button)
        {
          case SDL_BUTTON_LEFT:
              return MouseInput::LEFT;
              break;
          case SDL_BUTTON_RIGHT:
              return MouseInput::RIGHT;
              break;
          case SDL_BUTTON_MIDDLE:
              return MouseInput::MIDDLE;
              break;
        } 

#ifdef DEBUG
    	fprintf(stderr,"Unknown SDL mouse button.\n");
#endif

        return 0;
    }

    Key SDLInput::convertKeyCharacter(SDL_Keysym keysym)
    {
        int value = 0; 
        Key key;
    

        switch (keysym.sym)
        {
          case SDLK_TAB:
              value = Key::TAB;
              break;
          case SDLK_LALT:
              value = Key::LEFT_ALT;
              break;
          case SDLK_RALT:
              value = Key::RIGHT_ALT;
              break;
          case SDLK_LSHIFT:
              value = Key::LEFT_SHIFT;
              break;
          case SDLK_RSHIFT:
              value = Key::RIGHT_SHIFT;
              break;
          case SDLK_LCTRL:
              value = Key::LEFT_CONTROL;
              break;
          case SDLK_RCTRL:
              value = Key::RIGHT_CONTROL;
              break;
          case SDLK_BACKSPACE:
              value = Key::BACKSPACE;
              break;
          case SDLK_PAUSE:
              value = Key::PAUSE;
              break;
          case SDLK_SPACE:
              value = Key::SPACE;
              break;
          case SDLK_ESCAPE:
              value = Key::ESCAPE;
              break;
          case SDLK_DELETE:
              value = Key::DELETE;
              break;
          case SDLK_INSERT:
              value = Key::INSERT;
              break;
          case SDLK_HOME:
              value = Key::HOME;
              break;
          case SDLK_END:
              value = Key::END;
              break;
          case SDLK_PAGEUP:
              value = Key::PAGE_UP;
              break;
          case SDLK_PRINTSCREEN:
              value = Key::PRINT_SCREEN;
              break;
          case SDLK_PAGEDOWN:
              value = Key::PAGE_DOWN;
              break;
          case SDLK_F1:
              value = Key::F1;
              break;
          case SDLK_F2:
              value = Key::F2;
              break;
          case SDLK_F3:
              value = Key::F3;
              break;
          case SDLK_F4:
              value = Key::F4;
              break;
          case SDLK_F5:
              value = Key::F5;
              break;
          case SDLK_F6:
              value = Key::F6;
              break;
          case SDLK_F7:
              value = Key::F7;
              break;
          case SDLK_F8:
              value = Key::F8;
              break;
          case SDLK_F9:
              value = Key::F9;
              break;
          case SDLK_F10:
              value = Key::F10;
              break;
          case SDLK_F11:
              value = Key::F11;
              break;
          case SDLK_F12:
              value = Key::F12;
              break;
          case SDLK_F13:
              value = Key::F13;
              break;
          case SDLK_F14:
              value = Key::F14;
              break;
          case SDLK_F15:
              value = Key::F15;
              break;
          case SDLK_NUMLOCKCLEAR:
              value = Key::NUM_LOCK;
              break;
          case SDLK_CAPSLOCK:
              value = Key::CAPS_LOCK;
              break;
          case SDLK_SCROLLLOCK:
              value = Key::SCROLL_LOCK;
              break;
          case SDLK_LGUI:
              value = Key::LEFT_SUPER;
              break;
          case SDLK_RGUI:
              value = Key::RIGHT_SUPER;
              break;
          case SDLK_MODE:
              value = Key::ALT_GR;
              break;
          case SDLK_UP:
              value = Key::UP;
              break;
          case SDLK_DOWN:
              value = Key::DOWN;
              break;
          case SDLK_LEFT:
              value = Key::LEFT;
              break;
          case SDLK_RIGHT:
              value = Key::RIGHT;
              break;
          case SDLK_RETURN:
              value = Key::ENTER;
              break;
          case SDLK_KP_ENTER:
              value = Key::ENTER;
              break;

          default:
              value = keysym.sym;
              break;
        }

        if (!(keysym.mod & KMOD_NUM))
        {
            switch (keysym.sym)
            {
              case SDLK_KP_0:
                  value = Key::INSERT;
                  break;
              case SDLK_KP_1:
                  value = Key::END;
                  break;
              case SDLK_KP_2:
                  value = Key::DOWN;
                  break;
              case SDLK_KP_3:
                  value = Key::PAGE_DOWN;
                  break;
              case SDLK_KP_4:
                  value = Key::LEFT;
                  break;
              case SDLK_KP_5:
                  value = 0;
                  break;
              case SDLK_KP_6:
                  value = Key::RIGHT;
                  break;
              case SDLK_KP_7:
                  value = Key::HOME;
                  break;
              case SDLK_KP_8:
                  value = Key::UP;
                  break;
              case SDLK_KP_9:
                  value = Key::PAGE_UP;
                  break;
              default:
                  break;
            } 
        }

        key.setValue(value);
        key.setShiftPressed((keysym.mod & KMOD_SHIFT) != 0);
        key.setControlPressed((keysym.mod & KMOD_CTRL) != 0);
        key.setAltPressed((keysym.mod & KMOD_ALT) != 0);
        key.setMetaPressed((keysym.mod & KMOD_GUI) != 0);

        if (keysym.sym >= SDLK_KP_0 && keysym.sym <= SDLK_KP_EQUALS)
        {
            key.setNumericPad(true);
        }
    
        return key;    
    }
}
