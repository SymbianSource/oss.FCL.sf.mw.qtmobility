/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
// Copyright (c) 2008 Roberto Raggi <roberto.raggi@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef CPLUSPLUS_ARRAY_H
#define CPLUSPLUS_ARRAY_H

#include "CPlusPlusForwardDeclarations.h"
#include <new>

namespace CPlusPlus {

template <typename _Tp, int SEGMENT_SHIFT = 4>
class CPLUSPLUS_EXPORT Array
{
    Array(const Array &other);
    void operator =(const Array &other);

public:
    Array()
        : _segments(0),
          _allocatedSegments(0),
          _segmentCount(-1),
          _allocatedElements(0),
          _count(-1)
    { }

    ~Array()
    {
        if (_segments) {
            for (int index = 0; index <= _segmentCount; ++index) {
                delete[] (_segments[index] + (index << SEGMENT_SHIFT));
            }
            std::free(_segments);
        }
    }

    inline unsigned size() const
    { return _count + 1; }

    inline unsigned count() const
    { return _count + 1; }

    inline const _Tp &at(unsigned index) const
    { return _segments[index >> SEGMENT_SHIFT][index]; }

    inline const _Tp &operator[](unsigned index) const
    { return _segments[index >> SEGMENT_SHIFT][index]; }

    inline _Tp &operator[](unsigned index)
    { return _segments[index >> SEGMENT_SHIFT][index]; }

    void push_back(const _Tp &value)
    {
        if (++_count == _allocatedElements) {
            if (++_segmentCount == _allocatedSegments) {
                _allocatedSegments += 4;
                _segments = (_Tp **) std::realloc(_segments, _allocatedSegments * sizeof(_Tp *));
            }

            _Tp *segment = new _Tp[SEGMENT_SIZE];
            _segments[_segmentCount] = segment - (_segmentCount << SEGMENT_SHIFT);
            _allocatedElements += SEGMENT_SIZE;
        }

        _segments[_count >> SEGMENT_SHIFT][_count] = value;
    }

private:
    enum {
        SEGMENT_SIZE = 1 << SEGMENT_SHIFT
    };

    _Tp **_segments;
    int _allocatedSegments;
    int _segmentCount;

    int _allocatedElements;
    int _count;
};

} // end of namespace CPlusPlus


#endif // CPLUSPLUS_ARRAY_H
