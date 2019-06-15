/*
 *
 * Copyright (C) 2019  Miroslav Krajicek (https://github.com/kaajo).
 * All Rights Reserved.
 *
 * This file is part of CanObjects.
 *
 * CanObjects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU LGPL version 3 as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CanObjects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU LGPL version 3
 * along with CanObjects. If not, see <http://www.gnu.org/licenses/lgpl-3.0.txt>.
 *
 */

#ifndef CANBASE_GLOBAL_HPP
#define CANBASE_GLOBAL_HPP

#include <QtCore/qglobal.h>

#if defined(CANBASE_LIBRARY)
#  define CANBASESHARED_EXPORT Q_DECL_EXPORT
#else
#  define CANBASESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CANBASE_GLOBAL_HPP
