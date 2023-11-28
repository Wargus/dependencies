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

#include <allegro.h>

#include "guichan/allegro/allegroimageloader.hpp"
#include "guichan/exception.hpp"
#include "guichan/image.hpp"

namespace gcn
{
    AllegroImageLoader::AllegroImageLoader()
    {
        mBmp = NULL;
        mRawData = NULL;
    }
    
    AllegroImageLoader::~AllegroImageLoader()
    {
        if (mBmp != NULL)
        {
            destroy_bitmap(mBmp);
        }

        if (mRawData != NULL)
        {
            delete[] mRawData;
        }
    }
    
    void AllegroImageLoader::prepare(const std::string& filename)
    {
        if (mBmp != NULL)
        {
            throw GCN_EXCEPTION(std::string("Older image has not been finalized or discarded") + filename);
        }
        
#if !(ALLEGRO_VERSION == 4 && ALLEGRO_SUB_VERSION == 0)
        int colconv = get_color_conversion();
#endif

        set_color_conversion(COLORCONV_NONE);

        PALETTE pal;
        BITMAP *bmp = load_bitmap(filename.c_str(), pal);        
        
        if (bmp == NULL)
        {
            throw GCN_EXCEPTION(std::string("Unable to load: ") + filename);
        }

        mBmp = create_bitmap_ex(32, bmp->w, bmp->h);

        if (mBmp == NULL)
        {
            throw GCN_EXCEPTION(std::string("Not enough memory to load: ") + filename);
        }
        
        set_palette(pal);
        blit(bmp, mBmp, 0, 0, 0, 0, bmp->w, bmp->h);
        destroy_bitmap(bmp);
        
#if (ALLEGRO_VERSION == 4 && ALLEGRO_SUB_VERSION == 0)
        set_color_conversion(COLORCONV_TOTAL);
#else
        set_color_conversion(colconv);        
#endif
        
    }
    
    void AllegroImageLoader::free(Image* image)
    {
        BITMAP *bmp = (BITMAP *)image->_getData();
        destroy_bitmap(bmp);
    }
    
    void* AllegroImageLoader::getRawData()
    {
        // @todo do it!
        throw GCN_EXCEPTION("Not implemented yet");

        
        if (mBmp == NULL)
        {
            GCN_EXCEPTION("No image seems to be loaded");
        }
        
        return 0;
    }
    
    void* AllegroImageLoader::finalize()
    {
        if (mBmp == NULL)
        {
            GCN_EXCEPTION("No image seems to be loaded");
        }        
        
        BITMAP *bmp = create_bitmap(mBmp->w, mBmp->h);

        blit(mBmp, bmp, 0, 0, 0, 0, bmp->w, bmp->h);

        mBmp = NULL;

        if (mRawData != NULL)
        {
            delete[] mRawData;
            mRawData = NULL;            
        }
                
        return bmp;
    }
    
    void AllegroImageLoader::discard()
    {
        if (mBmp == NULL)
        {
            GCN_EXCEPTION("No image seems to be loaded");
        }                
        
        destroy_bitmap(mBmp);
        
        if (mRawData != NULL)
        {
            delete[] mRawData;
            mRawData = NULL;
        }
    }    
    
    int AllegroImageLoader::getHeight() const
    {
        if (mBmp == NULL)
        {
            GCN_EXCEPTION("No image seems to be loaded");
        }                        
        
        return mBmp->h;
    }
    
    int AllegroImageLoader::getWidth() const
    {
        if (mBmp == NULL)
        {
            GCN_EXCEPTION("No image seems to be loaded");
        }                
        
        return mBmp->w;
    }
    
    Color AllegroImageLoader::getPixel(int x, int y)
    {
        int c = getpixel(mBmp, x, y);

        return Color(getr32(c), getg32(c), getb32(c), geta(32));
    }
    
    void AllegroImageLoader::putPixel(int x, int y, const Color& color)
    {
        int c = makeacol_depth(32, color.r, color.g, color.b, color.a);

        putpixel(mBmp, x, y, c);
    }    
}
