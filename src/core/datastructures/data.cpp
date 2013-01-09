#include "inviwo/core/datastructures/data.h"

namespace inviwo {

    Data::Data() {}
    Data::~Data() {}

    void Data::clearRepresentations() {
        while (!representations_.empty()) {
            delete representations_.back();
            representations_.pop_back();
        }
    }

    void Data::copyRepresentations(Data* targetData) {
        targetData->representations_.clear();
        for(size_t i=0; i<representations_.size(); i++) {
            targetData->representations_.push_back(representations_[i]->clone());
        }
    }

    void Data::addRepresentation(DataRepresentation* representation) {
        representations_.push_back(representation);
    }

    /*---------------------------------------------------------------*/

    Data3D::Data3D() : PARENT(Data::TYPE3D(0)) {
        
    }

    Data3D::Data3D(Data::TYPE3D dim) : PARENT(dim) {

    }

    Data3D::~Data3D() {}

    ivec3 Data3D::getDimension() {
        return Data3D::PARENT::getDimension<ivec3,IVec3MetaData>(ivec3(0));
    }

    void Data3D::setDimension(ivec3 dim) {
         Data3D::PARENT::setDimension<ivec3,IVec3MetaData>(dim);
    }


    /*---------------------------------------------------------------*/

    Data2D::Data2D() : PARENT(Data::TYPE2D(0)) {

    }

    Data2D::Data2D(Data::TYPE2D dim) : PARENT(dim) {

    }

    Data2D::~Data2D() {}

    ivec2 Data2D::getDimension() {
        return Data2D::PARENT::getDimension<ivec2,IVec2MetaData>(ivec2(0));
    }

    void Data2D::setDimension(ivec2 dim) {
        Data2D::PARENT::setDimension<ivec2,IVec2MetaData>(dim);
    }


} // namespace
