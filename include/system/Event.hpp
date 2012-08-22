//Event.hpp

/*
 *   Copyright 2010-2011 Alexander Medvedev
 *
 *   This file is part of Hydra project.
 *   See <http://hydraproject.org.ua> for more info.
 *
 *   Hydra is free software: you can redistribute it and/or modify
 *   it under the terms of the MIT License:
 *   <http://www.opensource.org/licenses/mit-license.php>
 *   See notice at the end of this file.
 *
 *   Based on libSDL code.
 */



#ifndef EVENT_HPP__
#define EVENT_HPP__

/**
 * \struct hydra::system::Event
 * \brief Struct to store window event data
 *
 * May contain mouse motion event data, key press data and so on.
 * Also contains some enums to interpret data encoded in this structure.
 * It is based on SDL code.
 *
 * \todo Groupling (doxygen) does not work for enums. Fix it.
 *
 * \author A.V.Medvedev
 * \date 30.07.2010
 *
 */

#include "common/SharedPtr.hpp"

namespace hydra{
namespace system{

struct Event{

    ///represents types of event
    enum Type{
        ///this event is empty
        EMPTY = 0,
        
        ///window resize event
        RESIZE,

        ///window expose (must be redrawn) event
        EXPOSE,
        
        ///mouse moved
        MOUSE_MOTION,
        
        ///key has been pressed (keyboard or mouse)
        KEY_PRESS,
        
        ///key has been released (keyboard or mouse)
        KEY_RELEASE,

        ///mouse cursor leaves window area
        MOUSE_FOCUS_LEAVE,

        ///mouse cursor enteres window area
        MOUSE_FOCUS_GAIN,

        ///window is activated for input
        ACTIVATE,

        ///window is deactivated (probably by other window)
        DEACTIVATE,

        ///quit (you should handle this event or window will be 'unclosable')
        QUIT
    };

    ///keyboard modifiers
    enum KeyModifiers{
        ///none
        NONE = 0,

        ///caps lock is on
        CAPS_MASK = 1,

        ///num lock is on
        NUM_MASK = 2,

        ///left ALT key is pressed
        LALT_MASK = 4,

        ///right ALT key is pressed
        RALT_MASK = 8,

        ///left CTRL key is pressed
        LCTRL_MASK = 16,

        ///right CTRL is pressed
        RCTRL_MASK = 32,

        ///left shift is pressed
        LSHIFT_MASK = 64,

        ///right shift is pressed
        RSHIFT_MASK = 128,

        ///CTRL is pressed,
        CTRL_MASK = LCTRL_MASK | RCTRL_MASK,

        ///ALT is pressed
        ALT_MASK = LALT_MASK | RALT_MASK,

        ///shift is pressed
        SHIFT_MASK = LSHIFT_MASK | RSHIFT_MASK

    };

    /**
     * \brief Codes for keyboard and mouse buttons
     *
     * Most of the key codes are ASCII-mapped, so instead of Event::KEY_a
     * you may use 'a'.
     *
     * WORLD_x - are international virtual keycodes.
     */
    enum KeyCode{
        
        /* ASCII mapped keysyms
         *  The keyboard syms have been cleverly chosen to map to ASCII
         */
        //@{
        UNKNOWN     = 0,
        BACKSPACE   = 8,
        TAB         = 9,
        CLEAR       = 12,
        RETURN      = 13,
        PAUSE       = 19,
        ESCAPE      = 27,
        SPACE       = 32,
        EXCLAIM     = 33,
        QUOTEDBL    = 34,
        HASH        = 35,
        DOLLAR      = 36,
        AMPERSAND   = 38,
        QUOTE       = 39,
        LEFTPAREN   = 40,
        RIGHTPAREN  = 41,
        ASTERISK    = 42,
        PLUS        = 43,
        COMMA       = 44,
        MINUS       = 45,
        PERIOD      = 46,
        SLASH       = 47,
        NUM_0       = 48,
        NUM_1       = 49,
        NUM_2       = 50,
        NUM_3       = 51,
        NUM_4       = 52,
        NUM_5       = 53,
        NUM_6       = 54,
        NUM_7       = 55,
        NUM_8       = 56,
        NUM_9       = 57,
        COLON       = 58,
        SEMICOLON   = 59,
        LESS        = 60,
        EQUALS      = 61,
        GREATER     = 62,
        QUESTION    = 63,
        AT          = 64,
        
        
        LEFTBRACKET = 91,
        BACKSLASH   = 92,
        RIGHTBRACKET= 93,
        CARET       = 94,
        UNDERSCORE  = 95,
        BACKQUOTE   = 96,
        KEY_a       = 97,
        KEY_b       = 98,
        KEY_c       = 99,
        KEY_d       = 100,
        KEY_e       = 101,
        KEY_f       = 102,
        KEY_g       = 103,
        KEY_h       = 104,
        KEY_i       = 105,
        KEY_j       = 106,
        KEY_k       = 107,
        KEY_l       = 108,
        KEY_m       = 109,
        KEY_n       = 110,
        KEY_o       = 111,
        KEY_p       = 112,
        KEY_q       = 113,
        KEY_r       = 114,
        KEY_s       = 115,
        KEY_t       = 116,
        KEY_u       = 117,
        KEY_v       = 118,
        KEY_w       = 119,
        KEY_x       = 120,
        KEY_y       = 121,
        KEY_z       = 122,
        DELETE      = 127,
        //@}

        // International keyboard syms
        //@{
        WORLD_0         = 160,      
        WORLD_1         = 161,
        WORLD_2         = 162,
        WORLD_3         = 163,
        WORLD_4         = 164,
        WORLD_5         = 165,
        WORLD_6         = 166,
        WORLD_7         = 167,
        WORLD_8         = 168,
        WORLD_9         = 169,
        WORLD_10        = 170,
        WORLD_11        = 171,
        WORLD_12        = 172,
        WORLD_13        = 173,
        WORLD_14        = 174,
        WORLD_15        = 175,
        WORLD_16        = 176,
        WORLD_17        = 177,
        WORLD_18        = 178,
        WORLD_19        = 179,
        WORLD_20        = 180,
        WORLD_21        = 181,
        WORLD_22        = 182,
        WORLD_23        = 183,
        WORLD_24        = 184,
        WORLD_25        = 185,
        WORLD_26        = 186,
        WORLD_27        = 187,
        WORLD_28        = 188,
        WORLD_29        = 189,
        WORLD_30        = 190,
        WORLD_31        = 191,
        WORLD_32        = 192,
        WORLD_33        = 193,
        WORLD_34        = 194,
        WORLD_35        = 195,
        WORLD_36        = 196,
        WORLD_37        = 197,
        WORLD_38        = 198,
        WORLD_39        = 199,
        WORLD_40        = 200,
        WORLD_41        = 201,
        WORLD_42        = 202,
        WORLD_43        = 203,
        WORLD_44        = 204,
        WORLD_45        = 205,
        WORLD_46        = 206,
        WORLD_47        = 207,
        WORLD_48        = 208,
        WORLD_49        = 209,
        WORLD_50        = 210,
        WORLD_51        = 211,
        WORLD_52        = 212,
        WORLD_53        = 213,
        WORLD_54        = 214,
        WORLD_55        = 215,
        WORLD_56        = 216,
        WORLD_57        = 217,
        WORLD_58        = 218,
        WORLD_59        = 219,
        WORLD_60        = 220,
        WORLD_61        = 221,
        WORLD_62        = 222,
        WORLD_63        = 223,
        WORLD_64        = 224,
        WORLD_65        = 225,
        WORLD_66        = 226,
        WORLD_67        = 227,
        WORLD_68        = 228,
        WORLD_69        = 229,
        WORLD_70        = 230,
        WORLD_71        = 231,
        WORLD_72        = 232,
        WORLD_73        = 233,
        WORLD_74        = 234,
        WORLD_75        = 235,
        WORLD_76        = 236,
        WORLD_77        = 237,
        WORLD_78        = 238,
        WORLD_79        = 239,
        WORLD_80        = 240,
        WORLD_81        = 241,
        WORLD_82        = 242,
        WORLD_83        = 243,
        WORLD_84        = 244,
        WORLD_85        = 245,
        WORLD_86        = 246,
        WORLD_87        = 247,
        WORLD_88        = 248,
        WORLD_89        = 249,
        WORLD_90        = 250,
        WORLD_91        = 251,
        WORLD_92        = 252,
        WORLD_93        = 253,
        WORLD_94        = 254,
        WORLD_95        = 255,
        //@}
        
        //Numeric keypad
        //@{
        KP0     = 256,
        KP1     = 257,
        KP2     = 258,
        KP3     = 259,
        KP4     = 260,
        KP5     = 261,
        KP6     = 262,
        KP7     = 263,
        KP8     = 264,
        KP9     = 265,
        KP_PERIOD       = 266,
        KP_DIVIDE       = 267,
        KP_MULTIPLY     = 268,
        KP_MINUS        = 269,
        KP_PLUS         = 270,
        KP_ENTER        = 271,
        KP_EQUALS       = 272,

        UP              = 273,
        DOWN            = 274,
        RIGHT           = 275,
        LEFT            = 276,
        INSERT          = 277,
        HOME            = 278,
        END             = 279,
        PAGEUP          = 280,
        PAGEDOWN        = 281,
        //@}
        
        //Function keys
        //@{
        F1          = 282,
        F2          = 283,
        F3          = 284,
        F4          = 285,
        F5          = 286,
        F6          = 287,
        F7          = 288,
        F8          = 289,
        F9          = 290,
        F10         = 291,
        F11         = 292,
        F12         = 293,
        F13         = 294,
        F14         = 295,
        F15         = 296,
        //@}

        //Key state modifier keys
        //@{
        NUMLOCK     = 300,
        CAPSLOCK    = 301,
        SCROLLOCK   = 302,
        RSHIFT      = 303,
        LSHIFT      = 304,
        RCTRL       = 305,
        LCTRL       = 306,
        RALT        = 307,
        LALT        = 308,
        RMETA       = 309,
        LMETA       = 310,
        LSUPER      = 311,      ///< Left "Windows" key
        RSUPER      = 312,      ///< Right "Windows" key
        MODE        = 313,      ///< "Alt Gr" key
        COMPOSE     = 314,      ///< Multi-key compose key
        //@}

        //Miscellaneous function keys
        //@{
        HELP        = 315,
        PRINT       = 316,
        SYSREQ      = 317,
        BREAK       = 318,
        MENU        = 319,
        POWER       = 320,      ///< Power Macintosh power key
        EURO        = 321,      ///< Some european keyboards
        UNDO        = 322,       ///< Atari keyboard has Undo
        //@}
        
        //Mouse buttons
        //@{
        MOUSE_LEFT  = 400,
        MOUSE_RIGHT = 401,
        MOUSE_MIDDLE = 402
        //some other mouse buttons may be here...
        //@}
    };

    ///type of this event
    Event::Type type;

    ///some x coordinate (or width). Its meaning depends on event's type.
    int x;

    ///some y coordinate (or height). Its meaning depends on event's type.
    int y;

    ///key's code (for pressed or released key)
    Event::KeyCode key;
    
    ///keyboard modificator keys (like ctrl, alt, etc)
    Event::KeyModifiers mod;

};

///pointer to event
typedef common::SharedPtr<hydra::system::Event>::Type EventPtr;

} //system namespace

} //hydra

#endif

/*
 *   Copyright 2010-2011 Alexander Medvedev
 *
 *   Permission is hereby granted, free of charge, to any person 
 *   obtaining a copy of this software and associated documentation
 *   files (the "Software"), to deal in the Software without
 *   restriction, including without limitation the rightsto use, 
 *   copy, modify, merge, publish, distribute, sublicense, and/or
 *   sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following 
 *   conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 */
