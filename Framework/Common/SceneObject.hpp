#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "Guid.hpp"
#include "Image.hpp"
#include "portable.hpp"

namespace My {
    namespace details {
        constexpr int32_t i32(const char* s, int32_t v) {
            return *s ? i32(s+1, v * 256 + *s) : v;
        }
    }

    constexpr int32_t operator "" _i32(const char* s, size_t) {
        return details::i32(s, 0);
    }

    ENUM(SceneObjectType) {
        kSceneObjectTypeMesh    =   "MESH"_i32,
        kSceneObjectTypeMaterial=   "MATL"_i32,
        kSceneObjectTypeTexture =   "TXTU"_i32,
        kSceneObjectTypeLight   =   "LGHT"_i32,
        kSceneObjectTypeCamera  =   "CAMR"_i32,
        kSceneObjectTypeAnimator=   "ANIM"_i32,
        kSceneObjectTypeClip    =   "CLIP"_i32,
        kSceneObjectTypeVertexArray   =   "VARR"_i32,
        kSceneObjectTypeIndexArray    =   "VARR"_i32,
        kSceneObjectTypeGeometry =  "GEOM"_i32,
    };

    std::ostream& operator<<(std::ostream& out, SceneObjectType type)
    {
        int32_t n = static_cast<int32_t>(type);
        n = endian_net_unsigned_int<int32_t>(n);
        char* c = reinterpret_cast<char*>(&n);
         
        for (size_t i = 0; i < sizeof(int32_t); i++) {
            out << *c++;
        }

        return out;
    }

    using namespace xg;
    class BaseSceneObject
    {
        protected:
            Guid m_Guid;
            SceneObjectType m_Type;
        protected:
            // can only be used as base class
            BaseSceneObject(SceneObjectType type) : m_Type(type) { m_Guid = newGuid(); };
            BaseSceneObject(Guid& guid, SceneObjectType type) : m_Guid(guid), m_Type(type) {};
            BaseSceneObject(Guid&& guid, SceneObjectType type) : m_Guid(std::move(guid)), m_Type(type) {};
            BaseSceneObject(BaseSceneObject&& obj) : m_Guid(std::move(obj.m_Guid)), m_Type(obj.m_Type) {};
            BaseSceneObject& operator=(BaseSceneObject&& obj) { this->m_Guid = std::move(obj.m_Guid); this->m_Type = obj.m_Type; return *this; };
            
        private:
            // a type must be specified
            BaseSceneObject() = delete; 
            // can not be copied
            BaseSceneObject(BaseSceneObject& obj) = delete;
            BaseSceneObject& operator=(BaseSceneObject& obj) = delete;

        public:
            const Guid& GetGuid() const { return m_Guid; };
            const SceneObjectType GetType() const { return m_Type; };

        friend std::ostream& operator<<(std::ostream& out, const BaseSceneObject& obj)
        {
            out << "SceneObject" << std::endl;
            out << "-----------" << std::endl;
            out << "GUID: " << obj.m_Guid << std::endl;
            out << "Type: " << obj.m_Type << std::endl;

            return out;
        }
    };

    ENUM(VertexDataType) {
        kVertexDataTypeFloat1    = "FLT1"_i32,
        kVertexDataTypeFloat2    = "FLT2"_i32,
        kVertexDataTypeFloat3    = "FLT3"_i32,
        kVertexDataTypeFloat4    = "FLT4"_i32,
        kVertexDataTypeDouble1   = "DUB1"_i32,
        kVertexDataTypeDouble2   = "DUB2"_i32,
        kVertexDataTypeDouble3   = "DUB3"_i32,
        kVertexDataTypeDouble4   = "DUB3"_i32
    };

    class SceneObjectVertexArray : public BaseSceneObject
    {
        protected:
            std::string m_Attribute;
            uint32_t    m_MorphTargetIndex;
            VertexDataType m_DataType;

            void*      m_pDataFloat;

            size_t      m_szData;

        public:
            SceneObjectVertexArray(const char* attr, void* data, size_t data_size, VertexDataType data_type, uint32_t morph_index = 0) : BaseSceneObject(SceneObjectType::kSceneObjectTypeVertexArray), m_Attribute(attr), m_MorphTargetIndex(morph_index), m_DataType(data_type), m_pDataFloat(data), m_szData(data_size) {};
    };

    ENUM(IndexDataType) {
        kIndexDataTypeInt16 = "_I16"_i32,
        kIndexDataTypeInt32 = "_I32"_i32,
    };

    class SceneObjectIndexArray : public BaseSceneObject
    {
        protected:
            uint32_t    m_MaterialIndex;
            size_t      m_RestartIndex;
            IndexDataType m_DataType;

            void*       m_pData;

            size_t      m_szData;

        public:
            SceneObjectIndexArray(uint32_t material_index, IndexDataType data_type = IndexDataType::kIndexDataTypeInt16, uint32_t restart_index = 0) : BaseSceneObject(SceneObjectType::kSceneObjectTypeIndexArray), m_MaterialIndex(material_index), m_RestartIndex(restart_index), m_DataType(data_type) {};
    };

    class SceneObjectMesh : public BaseSceneObject
    {
        protected:
            std::vector<SceneObjectIndexArray>  m_IndexArray;
            std::vector<SceneObjectVertexArray> m_VertexArray;

            bool        m_bVisible;
            bool        m_bShadow;
            bool        m_bMotionBlur;
            
        public:
            SceneObjectMesh(bool visible = true, bool shadow = true, bool motion_blur = true) : BaseSceneObject(SceneObjectType::kSceneObjectTypeMesh), m_bVisible(visible), m_bShadow(shadow), m_bMotionBlur(motion_blur) {};
            void AddIndexArray(SceneObjectIndexArray&& array) { m_IndexArray.push_back(std::move(array)); };
            void AddVertxArray(SceneObjectVertexArray&& array) { m_VertexArray.push_back(std::move(array)); };
    };

    template <typename T>
    struct ParameterMap
    {
        bool bUsingSingleValue;

        union {
            T Value;
            Image* Map;
        };

        ParameterMap(T value) : bUsingSingleValue(true), Value(value) {};
    };

    typedef ParameterMap<Vector4f> Color;
    typedef ParameterMap<Vector3f> Normal;
    typedef ParameterMap<float>    Parameter;

    class SceneObjectMaterial : public BaseSceneObject
    {
        protected:
            Color       m_BaseColor;
            Parameter   m_Metallic;
            Parameter   m_Roughness;
            Normal      m_Normal;
            Parameter   m_Specular;
            Parameter   m_AmbientOcclusion;

        public:
            SceneObjectMaterial(Color base_color = Vector4f(1.0f), Parameter metallic = 0.0f, Parameter roughness = 0.0f, Normal normal = Vector3f(0.0f, 0.0f, 1.0f), Parameter specular = 0.0f, Parameter ao = 1.0f) : BaseSceneObject(SceneObjectType::kSceneObjectTypeMaterial), m_BaseColor(base_color), m_Metallic(metallic), m_Roughness(roughness), m_Normal(normal), m_Specular(specular), m_AmbientOcclusion(ao) {};
    };

    class SceneObjectGeometry : public BaseSceneObject
    {
        protected:
            std::vector<SceneObjectMesh> m_Mesh;

        public:
            void AddMesh(SceneObjectMesh&& mesh) { m_Mesh.push_back(std::move(mesh)); };
            SceneObjectGeometry() : BaseSceneObject(SceneObjectType::kSceneObjectTypeGeometry) {};
    };

    typedef float (*AttenFunc)(float /* Intensity */, float /* Distance */);

    class SceneObjectLight : public BaseSceneObject
    {
        protected:
            Color       m_LightColor;
            float       m_Intensity;
            AttenFunc   m_LightAttenuation;
            float       m_fNearClipDistance;
            float       m_fFarClipDistance;
            bool        m_bCastShadows;

        protected:
            // can only be used as base class of delivered lighting objects
            SceneObjectLight() : BaseSceneObject(SceneObjectType::kSceneObjectTypeLight), m_LightColor(0) {};
    };

    class SceneObjectOmniLight : public SceneObjectLight
    {
        public:
            using SceneObjectLight::SceneObjectLight;
    };

    class SceneObjectSpotLight : public SceneObjectLight
    {
        protected:
            float   m_fConeAngle;
            float   m_fPenumbraAngle;
        public:
            using SceneObjectLight::SceneObjectLight;
    };

    class SceneObjectCamera : public BaseSceneObject
    {
        protected:
            float m_fFov;
            float m_fNearClipDistance;
            float m_fFarClipDistance;
        public:
            SceneObjectCamera() : BaseSceneObject(SceneObjectType::kSceneObjectTypeCamera) {};
    };
}

