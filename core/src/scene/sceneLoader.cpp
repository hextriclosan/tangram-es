#include "sceneLoader.h"

#include "platform.h"
#include "scene.h"
#include "tileManager.h"
#include "view.h"
#include "lights.h"
#include "geoJsonSource.h"
#include "mvtSource.h"
#include "polygonStyle.h"
#include "polylineStyle.h"
#include "debugStyle.h"

#include "yaml-cpp/yaml.h"

using namespace YAML;


void SceneLoader::loadScene(const std::string& _file, Scene& _scene, TileManager& _tileManager, View& _view) {

    std::string configString = stringFromResource(_file.c_str());

    Node config = YAML::Load(configString);

    loadSources(config["sources"], _tileManager);
    loadLayers(config["layers"], _scene, _tileManager);
    loadCameras(config["cameras"], _view);
    loadLights(config["lights"], _scene);

}

glm::vec4 parseVec4(const Node& node) {
    glm::vec4 vec;
    int i = 0;
    for (auto it = node.begin(); it != node.end(); ++it) {
        if (i < 4) {
            vec[i++] = it->as<float>();
        } else {
            break;
        }
    }
    return vec;
}

glm::vec3 parseVec3(const Node& node) {
    glm::vec3 vec;
    int i = 0;
    for (auto it = node.begin(); it != node.end(); ++it) {
        if (i < 3) {
            vec[i++] = it->as<float>();
        } else {
            break;
        }
    }
    return vec;
}

void SceneLoader::loadSources(Node sources, TileManager& tileManager) {

    if(!sources) {
        logMsg("Warning: No source defined in the yaml scene configuration.\n");
        return;
    }

    for (auto it = sources.begin(); it != sources.end(); ++it) {

        const Node source = it->second;
        std::string name = it->first.as<std::string>();
        std::string type = source["type"].as<std::string>();
        std::string url = source["url"].as<std::string>();

        std::unique_ptr<DataSource> sourcePtr;

        if (type == "GeoJSONTiles") {
            sourcePtr = std::unique_ptr<DataSource>(new GeoJsonSource(name, url));
        } else if (type == "TopoJSONTiles") {
            // TODO
        } else if (type == "MVT") {
            sourcePtr = std::unique_ptr<DataSource>(new MVTSource(name, url));
        }

        if (sourcePtr) {
            tileManager.addDataSource(std::move(sourcePtr));
        }
    }

}

void SceneLoader::loadLights(Node lights, Scene& scene) {

    if (!lights) {

        // Add an ambient light if nothing else is specified
        std::unique_ptr<AmbientLight> amb(new AmbientLight("defaultLight"));
        amb->setAmbientColor({ .5f, .5f, .5f, 1.f });
        scene.addLight(std::move(amb));

        return;
    }

    for (auto it = lights.begin(); it != lights.end(); ++it) {

        const Node light = it->second;
        const std::string name = it->first.Scalar();
        const std::string type = light["type"].as<std::string>();

        std::unique_ptr<Light> lightPtr;

        if (type == "ambient") {

            lightPtr = std::unique_ptr<Light>(new AmbientLight(name));

        } else if (type == "directional") {

            DirectionalLight* dLightPtr = new DirectionalLight(name);
            Node direction = light["direction"];
            if (direction) {
                dLightPtr->setDirection(parseVec3(direction));
            }
            lightPtr = std::unique_ptr<Light>(dLightPtr);

        } else if (type == "point") {

            PointLight* pLightPtr = new PointLight(name);
            Node position = light["position"];
            if (position) {
                pLightPtr->setPosition(parseVec3(position));
            }
            Node radius = light["radius"];
            if (radius) {
                if (radius.size() > 1) {
                    pLightPtr->setRadius(radius[0].as<float>(), radius[1].as<float>());
                } else {
                    pLightPtr->setRadius(radius.as<float>());
                }
            }
            Node att = light["attenuation"];
            if (att) {
                pLightPtr->setAttenuation(att.as<float>());
            }
            lightPtr = std::unique_ptr<Light>(pLightPtr);

        } else if (type == "spotlight") {

            SpotLight* sLightPtr = new SpotLight(name);
            Node position = light["position"];
            if (position) {
                sLightPtr->setPosition(parseVec3(position));
            }
            Node direction = light["direction"];
            if (direction) {
                sLightPtr->setDirection(parseVec3(direction));
            }
            Node radius = light["radius"];
            if (radius) {
                if (radius.size() > 1) {
                    sLightPtr->setRadius(radius[0].as<float>(), radius[1].as<float>());
                } else {
                    sLightPtr->setRadius(radius.as<float>());
                }
            }
            Node angle = light["angle"];
            if (angle) {
                sLightPtr->setCutoffAngle(angle.as<float>());
            }
            Node exponent = light["exponent"];
            if (exponent) {
                sLightPtr->setCutoffExponent(exponent.as<float>());
            }

            lightPtr = std::unique_ptr<Light>(sLightPtr);

        }

        Node origin = light["origin"];
        if (origin) {
            const std::string originStr = origin.as<std::string>();
            if (originStr == "world") {
                lightPtr->setOrigin(LightOrigin::WORLD);
            } else if (originStr == "camera") {
                lightPtr->setOrigin(LightOrigin::CAMERA);
            } else if (originStr == "ground") {
                lightPtr->setOrigin(LightOrigin::GROUND);
            }
        }

        Node ambient = light["ambient"];
        if (ambient) {
            lightPtr->setAmbientColor(parseVec4(ambient));
        }

        Node diffuse = light["diffuse"];
        if (diffuse) {
            lightPtr->setDiffuseColor(parseVec4(diffuse));
        }

        Node specular = light["specular"];
        if (specular) {
            lightPtr->setSpecularColor(parseVec4(specular));
        }

        scene.addLight(std::move(lightPtr));

    }

}

void SceneLoader::loadCameras(Node cameras, View& view) {

    // To correctly match the behavior of the webGL library we'll need a place to store multiple view instances.
    // Since we only have one global view right now, we'll just apply the settings from the first active camera we find.

    for (auto it = cameras.begin(); it != cameras.end(); ++it) {

        const Node camera = it->second;

        const std::string type = camera["type"].as<std::string>();
        if (type == "perspective") {
            // The default camera
            Node fov = camera["fov"];
            if (fov) {
                // TODO
            }

            Node focal = camera["focal_length"];
            if (focal) {
                // TODO
            }

            Node vanishing = camera["vanishing_point"];
            if (vanishing) {
                // TODO
            }
        } else if (type == "isometric") {
            // TODO
            Node axis = camera["axis"];
            if (axis) {
                // TODO
            }
        } else if (type == "flat") {
            // TODO
        }

        double x = -74.00976419448854;
        double y = 40.70532700869127;
        float z = 16;

        Node position = camera["position"];
        if (position) {
            x = position[0].as<double>();
            y = position[1].as<double>();
            if (position.size() > 2) {
                z = position[2].as<float>();
            }
        }
        glm::dvec2 projPos = view.getMapProjection().LonLatToMeters(glm::dvec2(x, y));
        view.setPosition(projPos.x, projPos.y);

        Node zoom = camera["zoom"];
        if (zoom) {
            z = zoom.as<float>();
        }
        view.setZoom(z);

        Node active = camera["active"];
        if (active) {
            break;
        }

    }

}

void SceneLoader::loadLayers(Node layers, Scene& scene, TileManager& tileManager) {

    if (!layers) {
        return;
    }

    // Instantiate base styles
    auto polygonStyle = std::unique_ptr<PolygonStyle>(new PolygonStyle("polygons"));
    auto polylineStyle = std::unique_ptr<PolylineStyle>(new PolylineStyle("lines"));
    auto debugStyle = std::unique_ptr<DebugStyle>(new DebugStyle("debug"));

    // TODO: configure style properties in styles block
    polygonStyle->setLightingType(LightingType::vertex);
    polylineStyle->setLightingType(LightingType::vertex);

    for (auto layerIt = layers.begin(); layerIt != layers.end(); ++layerIt) {

        std::string name = layerIt->first.as<std::string>();
        Node drawGroup = layerIt->second["draw"];
        Node data = layerIt->second["data"];

        // TODO: handle data.source

        Node dataLayer = data["layer"];
        if (dataLayer) { name = dataLayer.as<std::string>(); }

        for (auto groupIt = drawGroup.begin(); groupIt != drawGroup.end(); ++groupIt) {

            StyleParams params;
            std::string styleName = groupIt->first.as<std::string>();
            Node styleProps = groupIt->second;

            Node order = styleProps["order"];
            if (order) { params.order = order.as<float>(); }

            Node color = styleProps["color"];
            if (color) {
                glm::vec4 c = parseVec4(color);
                params.color = (uint32_t(c.a * 255) << 24) +
                               (uint32_t(c.b * 255) << 16) +
                               (uint32_t(c.g * 255) << 8)  +
                               (uint32_t(c.r * 255));
                // TODO: color helper funtions
            }

            Node width = styleProps["width"];
            if (width) { params.width = width.as<float>(); }

            Node cap = styleProps["cap"];
            if (cap) {
                std::string capString = cap.as<std::string>();
                if (capString == "butt") { params.line.cap = CapTypes::BUTT; }
                else if (capString == "sqaure") { params.line.cap = CapTypes::SQUARE; }
                else if (capString == "round") { params.line.cap = CapTypes::ROUND; }
            }

            Node join = styleProps["join"];
            if (join) {
                std::string joinString = join.as<std::string>();
                if (joinString == "bevel") { params.line.join = JoinTypes::BEVEL; }
                else if (joinString == "miter") { params.line.join = JoinTypes::MITER; }
                else if (joinString == "round") { params.line.join = JoinTypes::ROUND; }
            }

            Node outline = styleProps["outline"];
            if (outline) {

                params.outline.on = true;

                Node color = outline["color"];
                if (color) {
                    glm::vec4 c = parseVec4(color);
                    params.outline.color = (uint32_t(c.a * 255) << 24) +
                    (uint32_t(c.b * 255) << 16) +
                    (uint32_t(c.g * 255) << 8)  +
                    (uint32_t(c.r * 255));
                    // TODO: color helper funtions
                }

                Node width = outline["width"];
                if (width) { params.outline.width = width.as<float>(); }

                Node cap = outline["cap"];
                if (cap) {
                    std::string capString = cap.as<std::string>();
                    if (capString == "butt") { params.outline.line.cap = CapTypes::BUTT; }
                    else if (capString == "sqaure") { params.outline.line.cap = CapTypes::SQUARE; }
                    else if (capString == "round") { params.outline.line.cap = CapTypes::ROUND; }
                }

                Node join = outline["join"];
                if (join) {
                    std::string joinString = join.as<std::string>();
                    if (joinString == "bevel") { params.outline.line.join = JoinTypes::BEVEL; }
                    else if (joinString == "miter") { params.outline.line.join = JoinTypes::MITER; }
                    else if (joinString == "round") { params.outline.line.join = JoinTypes::ROUND; }
                }

            }

            // match to built-in styles
            if (styleName == "polygons") {
                polygonStyle->addLayer({ name, params });
            } else if (styleName == "lines") {
                polylineStyle->addLayer({ name, params });
            } else if (styleName == "text") {
                // TODO
            }

        }

    }

    scene.addStyle(std::move(polygonStyle));
    scene.addStyle(std::move(polylineStyle));
    scene.addStyle(std::move(debugStyle));

    // tileManager isn't used yet, but we'll need it soon to get the list of data sources

}
