/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/
#pragma once

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/base/algorithm/mesh/axisalignedboundingbox.h>

namespace inviwo {

namespace camerautil {

enum class Side { XNegative, XPositive, YNegative, YPositive, ZNegative, ZPositive };

/**
 * Setup the camera parameters such that the whole boundingBox spaned by basis and offset
 * will be inside the view frustum.
 *
 * @param cam       the Camera to update
 * @param boundingBox the basis and offset of the bounding box that will fit inside the new
 * view frustum
 * @param side       this side of the bounding box will be facing the camera afterward
 * @param fitRatio   determines the spacing between volume and the boundaries of the view frustum. A
 *                   fit ratio of 1 means a perfect fit, no space between frustum and volume. The
 * aspect ratio is taken into account.
 * @param setNearFar the camera's new/far clip ranges are updated if true @see setCameraNearFar
 * @param setLookRanges   the camera's look-to/look-from ranges are updated if true @see
 * setCameraLookRanges
 */
IVW_MODULE_BASE_API void setCameraView(CameraProperty &cam, const mat4 &boundingBox, Side side,
                                       float fitRatio = 1.05f, bool setNearFar = false,
                                       bool setLookRanges = false);

/**
 * Setup the camera parameters such that the all input meshes will be inside the view frustum.
 * @see setCameraView
 */
IVW_MODULE_BASE_API void setCameraView(CameraProperty &cam,
                                       const std::vector<std::shared_ptr<const Mesh>> &meshes,
                                       Side side, float fitRatio = 1.05f, bool setNearFar = false,
                                       bool setLookRanges = false);

/**
 * Setup the camera parameters such that the given mesh will be completely inside the view frustum.
 * @see setCameraView
 */
IVW_MODULE_BASE_API void setCameraView(CameraProperty &cam, const Mesh &meshe, Side side,
                                       float fitRatio = 1.05f, bool setNearFar = false,
                                       bool setLookRanges = false);

/**
 * Set the ranges of the look to and look from properties of the camera. Will center around the mid
 * point of dataToWorld. The lookTo will be set to ranges to stay within the volume. The lookFrom
 * will be set to the mid point +- the basis vector of boundingBox times the zoom factor. That is
 * a zoom factor of 25 will allow to zoom out to a distance of "25 volumes".
 *
 * @param cam           camera to update
 * @param boundingBox  basis and offset of the bounding box used to determinte the ranges
 * @param maxZoomFactor determines how far away from the volume the user will be able to zoom out.
 */
IVW_MODULE_BASE_API void setCameraLookRanges(CameraProperty &cam, const mat4 &boundingBox,
                                             float maxZoomFactor = 25.f);

/**
 * Computes appropriate near and far clip distances for the given bounding box and zoom factor.
 * Makes sure that the far plane is distant enough to avoid clipping given to current zoomfactor.
 * @see setCameraLookRanges
 */
IVW_MODULE_BASE_API std::pair<float, float> computeCameraNearFar(
    const mat4 &boundingBox, float maxZoomFactor = 25.f, float nearFarRatio = 1.f / 10000.f);

/**
 * Sets the near and far clip distances of the camera based on the given bounding volume and zoom
 * factor. Ensures that the far plane is distant enough to avoid clipping given to current
 * zoomfactor.
 * @see computeCameraNearFar
 * @see setCameraLookRanges
 */
IVW_MODULE_BASE_API void setCameraNearFar(CameraProperty &cam, const mat4 &boundingBox,
                                          float maxZoomFactor = 25.f,
                                          float nearFarRatio = 1.f / 10000.f);

/**
 * Helper class for wrapping properties and setting up their callbacks to update the given camera.
 * Can be used together with mesh ports (both single, multi and flat) and volume ports.
 * @see setCameraView
 * @see setCameraLookRanges
 * @see setCameraNearFar
 */
class IVW_MODULE_BASE_API FitCameraPropertiesHelper {
    FitCameraPropertiesHelper(std::string identifier, std::string displayName);

public:
    FitCameraPropertiesHelper(std::string identifier, std::string displayName,
                              CameraProperty &camera, VolumeInport &volumePort);

    template <size_t N, bool Flat>
    FitCameraPropertiesHelper(std::string identifier, std::string displayName,
                              CameraProperty &camera, DataInport<Mesh, N, Flat> &meshPort);

    CompositeProperty &getCompositeProperty() { return composite_; }

private:
    CompositeProperty composite_;
    CompositeProperty lookAt_;
    CompositeProperty lookAtSettings_;
    ButtonProperty flipUp_;
    BoolProperty updateNearFar_;
    BoolProperty updateLookRanges_;
    FloatProperty fittingRatio_;
    ButtonProperty xNegative_;
    ButtonProperty xPositive_;
    ButtonProperty yNegative_;
    ButtonProperty yPositive_;
    ButtonProperty zNegative_;
    ButtonProperty zPositive_;

    ButtonProperty setNearFarButton_;
    ButtonProperty setLookRangesButton_;

    template <typename Callback, typename Port>
    void init(CameraProperty &camera, Callback getToWorld, Port &port);
};

template <size_t N, bool Flat>
FitCameraPropertiesHelper::FitCameraPropertiesHelper(std::string identifier,
                                                     std::string displayName,
                                                     CameraProperty &camera,
                                                     DataInport<Mesh, N, Flat> &meshPort)
    : FitCameraPropertiesHelper(identifier, displayName) {
    auto getToWorld = [mp = &meshPort] {
        auto minMax = meshutil::axisAlignedBoundingBox(mp->getVectorData());
        auto m = glm::scale(minMax.second - minMax.first);
        m[3] = vec4(minMax.first, 1.0f);
        return m;
    };

    init(camera, getToWorld, meshPort);
}

template <typename Callback, typename Port>
void FitCameraPropertiesHelper::init(CameraProperty &camera, Callback getBoundingBox, Port &port) {

    auto set = [getBoundingBox, p = &port, cam = &camera, this](auto side) {
        if (p->hasData()) {
            setCameraView(*cam, getBoundingBox(), side, fittingRatio_.get(), updateNearFar_.get(),
                          updateLookRanges_.get());
        }
    };

    xNegative_.onChange([set] { set(Side::XNegative); });
    xPositive_.onChange([set] { set(Side::XPositive); });

    yNegative_.onChange([set] { set(Side::YNegative); });
    yPositive_.onChange([set] { set(Side::YPositive); });

    zNegative_.onChange([set] { set(Side::ZNegative); });
    zPositive_.onChange([set] { set(Side::ZPositive); });

    flipUp_.onChange([cam = &camera]() { cam->setLookUp(-cam->getLookUp()); });

    setNearFarButton_.onChange([getBoundingBox, p = &port, cam = &camera] {
        if (p->hasData()) {
            setCameraNearFar(*cam, getBoundingBox());
        }
    });
    setLookRangesButton_.onChange([getBoundingBox, p = &port, cam = &camera] {
        if (p->hasData()) {
            setCameraLookRanges(*cam, getBoundingBox());
        }
    });
}

}  // namespace camerautil

}  // namespace inviwo