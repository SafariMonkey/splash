/*
 * Copyright (C) 2016 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @mesh_bezierPatch.h
 * The Mesh_BezierPatch_BezierPatch class
 */

#ifndef SPLASH_MESH_BEZIERPATCH_H
#define SPLASH_MESH_BEZIERPATCH_H

#include <chrono>
#include <memory>
#include <mutex>
#include <vector>
#include <glm/glm.hpp>

#include "config.h"

#include "coretypes.h"
#include "basetypes.h"
#include "mesh.h"

namespace Splash {

class Mesh_BezierPatch : public Mesh
{
    public:
        /**
         * Constructor
         */
        Mesh_BezierPatch();
        Mesh_BezierPatch(bool linkedToWorld); //< This constructor is used if the object is linked to a World counterpart

        /**
         * Destructor
         */
        virtual ~Mesh_BezierPatch();

        /**
         * No copy constructor, but a copy operator
         */
        Mesh_BezierPatch(const Mesh_BezierPatch&) = delete;
        Mesh_BezierPatch& operator=(const Mesh_BezierPatch&) = delete;
        Mesh_BezierPatch& operator=(Mesh_BezierPatch&&) = default;

        /**
         * Get the list of the control points
         */
        std::vector<glm::vec2> getControlPoints() const {return _patch.vertices;}

        /**
         * Select the bezier mesh or the control points as the mesh to output
         */
        void switchMeshes(bool control);

        /**
         * Update the content of the mesh
         */
        virtual void update();

    private:
        struct Patch
        {
            glm::ivec2 size {0, 0};
            std::vector<glm::vec2> vertices {};
            std::vector<glm::vec2> uvs {};
        };

        Patch _patch {};
        int _patchResolution {64};

        bool _patchUpdated {true};
        MeshContainer _bezierControl;
        MeshContainer _bezierMesh;

        // Factorial
        inline int32_t factorial(int32_t i)
        {
            return (i == 0 || i == 1) ? 1 : factorial(i - 1) * i;
        }

        // "Semi" factorial
        inline int32_t factorial(int32_t n, int32_t k)
        {
            int32_t res = 1;
            for (int32_t i = n - k + 1; i <= n; ++i)
                res *= i;
            return res;
        }

        // Binomial coefficient
        inline int32_t binomialCoeff(int32_t n, int32_t i)
        {
            if (n < i)
                return 0;
            return factorial(n, i) / factorial(i);
        }

        void init();

        /**
         * Create a patch
         */
        void createPatch(int width = 4, int height = 4);
        void createPatch(Patch& patch);

        /**
         * Update the underlying mesh from the patch control points
         */
        void updatePatch();
        
        /**
         * Register new functors to modify attributes
         */
        void registerAttributes();
};

} // end of namespace

#endif // SPLASH_MESH_BEZIERPATCH_H