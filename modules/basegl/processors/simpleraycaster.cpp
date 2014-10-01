/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.6b
 *
 * Copyright (c) 2012-2014 Inviwo Foundation
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
 * Main file authors: Timo Ropinski, Erik Sund�n
 *
 *********************************************************************************/

#include "simpleraycaster.h"
#include <inviwo/core/io/serialization/ivwserialization.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/shaderutils.h>
#include <modules/opengl/volumeutils.h>

namespace inviwo {

ProcessorClassIdentifier(SimpleRaycaster,  "org.inviwo.SimpleRaycaster");
ProcessorDisplayName(SimpleRaycaster,  "Simple Raycaster");
ProcessorTags(SimpleRaycaster, Tags::GL);
ProcessorCategory(SimpleRaycaster, "Volume Rendering");
ProcessorCodeState(SimpleRaycaster, CODE_STATE_STABLE);

SimpleRaycaster::SimpleRaycaster()
    : Processor()
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport", &entryPort_, COLOR_DEPTH)
    , transferFunction_("transferFunction", "Transfer function", TransferFunction(), &volumePort_)
    , channel_("channel", "Render Channel")
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera")
    , lighting_("lighting", "Lighting") {
    
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    
    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();
    
    volumePort_.onChange(this, &SimpleRaycaster::onVolumeChange);

    addProperty(channel_);
    addProperty(transferFunction_);
    addProperty(raycasting_);
    addProperty(camera_);
    addProperty(lighting_);
}

SimpleRaycaster::~SimpleRaycaster() {
}

void SimpleRaycaster::initialize() {
    Processor::initialize();
    shader_ = new Shader("raycasting.frag", false);
    initializeResources();
}

void SimpleRaycaster::deinitialize() {
    if (shader_) delete shader_;
    shader_ = NULL;
    Processor::deinitialize();
}

void SimpleRaycaster::initializeResources() {
    util::glAddShaderDefines(shader_, raycasting_);
    util::glAddShaderDefines(shader_, camera_);
    util::glAddShaderDefines(shader_, lighting_);
    shader_->build();
}

void SimpleRaycaster::onVolumeChange(){
    if (volumePort_.hasData()){
        int channels = volumePort_.getData()->getDataFormat()->getComponents();

        if(channels == static_cast<int>(channel_.size()))
            return;
        
        channel_.clearOptions();
        for (int i = 0; i < channels; i++) {
            std::stringstream ss;
            ss << "Channel " << i;
            channel_.addOption(ss.str() , ss.str(), i);
        }
        channel_.setCurrentStateAsDefault();
    }
}

void SimpleRaycaster::process() {
    TextureUnit tfUnit, entryColorUnit, entryDepthUnit, exitColorUnit, exitDepthUnit, volUnit;
    util::glBindTexture(transferFunction_, tfUnit);
    util::glBindTextures(entryPort_, entryColorUnit, entryDepthUnit);
    util::glBindTextures(exitPort_, exitColorUnit, exitDepthUnit);
    util::glBindTexture(volumePort_, volUnit);

    util::glActivateAndClearTarget(outport_);
    shader_->activate();

    util::glSetShaderUniforms(shader_, outport_, "outportParameters_");
    shader_->setUniform("transferFunc_", tfUnit.getUnitNumber());
    shader_->setUniform("entryColorTex_", entryColorUnit.getUnitNumber());
    shader_->setUniform("entryDepthTex_", entryDepthUnit.getUnitNumber());   
    util::glSetShaderUniforms(shader_, entryPort_, "entryParameters_");
    shader_->setUniform("exitColorTex_", exitColorUnit.getUnitNumber());
    shader_->setUniform("exitDepthTex_", exitDepthUnit.getUnitNumber());
    util::glSetShaderUniforms(shader_, exitPort_, "exitParameters_");     
    shader_->setUniform("channel_", channel_.getSelectedValue());
    shader_->setUniform("volume_", volUnit.getUnitNumber());    
    util::glSetShaderUniforms(shader_, volumePort_, "volumeParameters_");
    util::glSetShaderUniforms(shader_, raycasting_);
    util::glSetShaderUniforms(shader_, camera_, "camera_");
    util::glSetShaderUniforms(shader_, lighting_, "light_");

    util::glSingleDrawImagePlaneRect();

    shader_->deactivate();
    util::glDeactivateCurrentTarget();
}

void SimpleRaycaster::deserialize(IvwDeserializer& d) {
    NodeVersionConverter<SimpleRaycaster> tvc(this, &SimpleRaycaster::fixNetwork);
    d.convertVersion(&tvc);
    Processor::deserialize(d);
}

bool SimpleRaycaster::fixNetwork(TxElement* node) {
    TxElement* p = util::xmlGetElement(node, "Properties/Property&type=OptionPropertyString&identifier=shadingMode");
    if(p) p->SetAttribute("type", "OptionPropertyInt");
    return true;
}

} // namespace
