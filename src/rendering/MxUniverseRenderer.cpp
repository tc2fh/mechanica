/*
    This file is part of Mechanica.

    Based on Magnum example

    Original authors — credit is appreciated but not required:

        2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 —
            Vladimír Vondruš <mosra@centrum.cz>
        2019 — Nghia Truong <nghiatruong.vn@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
 */


#include "MxSimulator.h"

#include <Corrade/Utility/Assert.h>
#include <Corrade/Containers/ArrayView.h>
#include <rendering/MxUniverseRenderer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/Shaders/Generic.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Trade/MeshData.h>

#include <Magnum/Animation/Easing.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Image.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Version.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Math/FunctionsBatch.h>

#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Icosphere.h>

#include <Magnum/Math/Vector4.h>


#include <rendering/WireframeObjects.h>
#include <rendering/MxStyle.hpp>

#include <MxUtil.h>
#include <MxLogger.h>
#include <mx_error.h>

#include <assert.h>
#include <iostream>
#include <stdexcept>



using namespace Magnum::Math::Literals;

MxUniverseRenderer::MxUniverseRenderer(const MxSimulator_Config &conf, MxWindow *win):
    window{win}, 
    _zoomRate(0.05), 
    _spinRate{0.01*M_PI}, 
    _moveRate{0.01}
{
    Log(LOG_DEBUG) << "Creating MxUniverseRenderer";

    //GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    Log(LOG_DEBUG) << "clip planes: " << conf.clipPlanes.size();
    
    if(conf.clipPlanes.size() > 0) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance0);
    }
    if(conf.clipPlanes.size() > 1) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance1);
    }
    if(conf.clipPlanes.size() > 2) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance2);
    }
    if(conf.clipPlanes.size() > 3) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance3);
    }
    if(conf.clipPlanes.size() > 4) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance4);
    }
    if(conf.clipPlanes.size() > 5) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance5);
    }
    if(conf.clipPlanes.size() > 6) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance6);
    }
    if(conf.clipPlanes.size() > 7) {
        GL::Renderer::enable(GL::Renderer::Feature::ClipDistance7);
    }
    if(conf.clipPlanes.size() > 8) {
        mx_exp(std::invalid_argument("only up to 8 clip planes supported"));
    }
    
    GL::Renderer::setDepthFunction(GL::Renderer::StencilFunction::Less);
    
    GL::Renderer::setClearColor(Color3{0.35f});
    
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    
    GL::Renderer::setBlendFunction(
       GL::Renderer::BlendFunction::SourceAlpha, /* or SourceAlpha for non-premultiplied */
       GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    /* Loop at 60 Hz max */
    glfwSwapInterval(1);

    MxVector3f origin = MxUniverse::origin();
    MxVector3f dim = MxUniverse::dim();

    center = (dim + origin) / 2.;

    sideLength = dim.max();
    
    MxVector3i size = {(int)std::ceil(dim[0]), (int)std::ceil(dim[1]), (int)std::ceil(dim[2])};

    /* Set up the camera */
    {
        /* Setup the arcball after the camera objects */
        const MxVector3f eye = MxVector3f(0.5f * sideLength, -2.2f * sideLength, 1.1f * sideLength);
        const MxVector3f center{0.f, 0.f, -0.1f * sideLength};
        const MxVector3f up = Vector3::zAxis();
        
        _eye = eye;
        _center = center;
        _up = up;
        const MxVector2i viewportSize = win->windowSize();

        _arcball = new Magnum::Mechanica::ArcBallCamera(eye, center, up, 45.0_degf,
            viewportSize, win->framebuffer().viewport().size());
    }

    /* Setup ground grid */
    
    // makes a grid and scene box. Both of these get made with extent
    // of {-1, 1}, thus, have a size of 2x2x2, so the transform for these
    // needs to cut them in half.
    gridMesh = MeshTools::compile(Primitives::grid3DWireframe({9, 9}));
    sceneBox = MeshTools::compile(Primitives::cubeWireframe());
    gridModelView = Matrix4::scaling({size[0]/2.f, size[1]/2.f, size[2]/2.f});

    setModelViewTransform(Matrix4::translation(-center));
    
    // set up the sphere rendering...
    sphereShader = Shaders::MxPhong {
        Shaders::MxPhong::Flag::VertexColor |
        Shaders::MxPhong::Flag::InstancedTransformation,
        1,                                                // light count
        (unsigned)conf.clipPlanes.size()                  // clip plane count
    };
    
    sphereInstanceBuffer = GL::Buffer{};

    largeSphereInstanceBuffer = GL::Buffer{};
    
    cuboidInstanceBuffer = GL::Buffer();
    
    cuboidMesh = MeshTools::compile(Primitives::cubeSolid());
    
    // setup bonds mesh, shader and vertex buffer.
    bondsMesh = GL::Mesh{};
    bondsVertexBuffer = GL::Buffer{};
    bondsMesh.setPrimitive(MeshPrimitive::Lines);
    
    // vertex colors for bonds shader
    flatShader = Shaders::Flat3D{Shaders::Flat3D::Flag::VertexColor };
    
    wireframeShader = Shaders::Flat3D{};
    
    bondsMesh.addVertexBuffer(bondsVertexBuffer, 0,
                              Shaders::Flat3D::Position{},
                              Shaders::Flat3D::Color3{});
    
    
    sphereMesh = MeshTools::compile(Primitives::icosphereSolid(2));

    largeSphereMesh = MeshTools::compile(Primitives::icosphereSolid(4));
    
    sphereMesh.addVertexBufferInstanced(sphereInstanceBuffer, 1, 0,
        Shaders::Phong::TransformationMatrix{},
        Shaders::Phong::NormalMatrix{},
        Shaders::Phong::Color4{});
    
    largeSphereMesh.addVertexBufferInstanced(largeSphereInstanceBuffer, 1, 0,
        Shaders::Phong::TransformationMatrix{},
        Shaders::Phong::NormalMatrix{},
        Shaders::Phong::Color4{});
    
    cuboidMesh.addVertexBufferInstanced(cuboidInstanceBuffer, 1, 0,
        Shaders::Phong::TransformationMatrix{},
        Shaders::Phong::NormalMatrix{},
        Shaders::Phong::Color4{});

    // setup up lighting properties. TODO: move these to style
    sphereShader.setShininess(2000.0f)
        .setLightPositions({{-20, 40, 20, 0.f}})
        .setLightColors({Magnum::Color3{0.9, 0.9, 0.9}})
        .setShininess(100)
        .setAmbientColor({0.4, 0.4, 0.4, 1})
        .setDiffuseColor({1, 1, 1, 0})
        .setSpecularColor({0.2, 0.2, 0.2, 0});
    
    _clipPlanes = std::vector<Magnum::Vector4>(conf.clipPlanes.size());
    for(int i = 0; i < conf.clipPlanes.size(); ++i) {
        Log(LOG_DEBUG) << "clip plane " << i << ": " << conf.clipPlanes[i];
        _clipPlanes[i] = conf.clipPlanes[i];
        sphereShader.setclipPlaneEquation(i, _clipPlanes[i]);
    }
    
    // we resize instances all the time.
    sphereMesh.setInstanceCount(0);
    largeSphereMesh.setInstanceCount(0);
    cuboidMesh.setInstanceCount(0);

    angleRenderer.start(conf.clipPlanes);
    dihedralRenderer.start(conf.clipPlanes);
    arrowRenderer.start(conf.clipPlanes);
}

static inline int render_particle(SphereInstanceData* pData, int i, MxParticle *p, space_cell *c) {

    MxParticleType *type = &_Engine.types[p->typeId];
    MxStyle *style = p->style ? p->style : type->style;
    
    if(style->flags & STYLE_VISIBLE) {
    
        Magnum::Vector3 position = {
            (float)(c->origin[0] + p->x[0]),
            (float)(c->origin[1] + p->x[1]),
            (float)(c->origin[2] + p->x[2])
        };
        
        float radius = p->flags & PARTICLE_CLUSTER ? 0 : p->radius;
        pData[i].transformationMatrix =
            Matrix4::translation(position) * Matrix4::scaling(Vector3{radius});
        pData[i].normalMatrix =
            pData[i].transformationMatrix.normalMatrix();
        pData[i].color = style->map_color(p);
        return 1;
    }
    
    return 0;
}

static inline int render_cuboid(CuboidInstanceData* pData, int i, MxCuboid *p, double *origin) {

    if(true) {
    
        Magnum::Vector3 position = {
            (float)(origin[0] + p->x[0]),
            (float)(origin[1] + p->x[1]),
            (float)(origin[2] + p->x[2])
        };
        
        Matrix4 translateRotate = Matrix4::from(p->orientation.toMatrix(), position);
        
        pData[i].transformationMatrix = translateRotate * Matrix4::scaling(0.5 * p->size);
        
        pData[i].normalMatrix =
            pData[i].transformationMatrix.normalMatrix();
        pData[i].color = Color4::red();
        return 1;
    }
    
    return 0;
}

static inline int render_bond(BondsInstanceData* bondData, int i, MxBond *bond) {

    if(!(bond->flags & BOND_ACTIVE)) 
        return 0;

    Magnum::Vector3 *color = &bond->style->color;
    MxParticle *pi = _Engine.s.partlist[bond->i];
    MxParticle *pj = _Engine.s.partlist[bond->j];
    
    double *oj = _Engine.s.celllist[pj->id]->origin;
    Magnum::Vector3 pj_origin = {static_cast<float>(oj[0]), static_cast<float>(oj[1]), static_cast<float>(oj[2])};
    
    int shift[3];
    Magnum::Vector3 pix;
    
    int *loci = _Engine.s.celllist[ bond->i ]->loc;
    int *locj = _Engine.s.celllist[ bond->j ]->loc;
    
    for ( int k = 0 ; k < 3 ; k++ ) {
        shift[k] = loci[k] - locj[k];
        if ( shift[k] > 1 )
            shift[k] = -1;
        else if ( shift[k] < -1 )
            shift[k] = 1;
        pix[k] = pi->x[k] + _Engine.s.h[k]* shift[k];
    }
                    
    bondData[i].position = pix + pj_origin;
    bondData[i].color = *color;
    bondData[i+1].position = pj->position + pj_origin;
    bondData[i+1].color = *color;
    return 2;
}


template<typename T>
MxUniverseRenderer& MxUniverseRenderer::draw(T& camera,
        const MxVector2i& viewportSize) {

    // the incomprehensible template madness way of doing things.
    // Containers::ArrayView<const float> data(reinterpret_cast<const float*>(&_points[0]), _points.size() * 3);
    // _bufferParticles.setData(data);
    
    WallTime wt;
    
    PerformanceTimer t1(engine_timer_render);
    PerformanceTimer t2(engine_timer_render_total);
    
    _dirty = false;

    sphereMesh.setInstanceCount(_Engine.s.nr_visible_parts);
    largeSphereMesh.setInstanceCount(_Engine.s.nr_visible_large_parts);
    cuboidMesh.setInstanceCount(_Engine.s.nr_visible_cuboids);

    // invalidate / resize the buffer
    sphereInstanceBuffer.setData({NULL,
        _Engine.s.nr_visible_parts * sizeof(SphereInstanceData)},
            GL::BufferUsage::DynamicDraw);

    largeSphereInstanceBuffer.setData({NULL,
        _Engine.s.nr_visible_large_parts * sizeof(SphereInstanceData)},
            GL::BufferUsage::DynamicDraw);
    
    cuboidInstanceBuffer.setData({NULL,
        _Engine.s.nr_visible_cuboids * sizeof(CuboidInstanceData)},
            GL::BufferUsage::DynamicDraw);
    
    // get pointer to data, give me the damned bytes
    SphereInstanceData* pData = (SphereInstanceData*)(void*)sphereInstanceBuffer.map(0,
            _Engine.s.nr_visible_parts * sizeof(SphereInstanceData),
            GL::Buffer::MapFlag::Write|GL::Buffer::MapFlag::InvalidateBuffer);

    int i = 0;
    for (int cid = 0 ; cid < _Engine.s.nr_cells ; cid++ ) {
        for (int pid = 0 ; pid < _Engine.s.cells[cid].count ; pid++ ) {
            MxParticle *p  = &_Engine.s.cells[cid].parts[pid];
            i += render_particle(pData, i, p, &_Engine.s.cells[cid]);
        }
    }
    assert(i == _Engine.s.nr_visible_parts);
    sphereInstanceBuffer.unmap();

    // get pointer to data, give me the damned bytes
    SphereInstanceData* pLargeData = (SphereInstanceData*)(void*)largeSphereInstanceBuffer.map(0,
            _Engine.s.nr_visible_large_parts * sizeof(SphereInstanceData),
            GL::Buffer::MapFlag::Write|GL::Buffer::MapFlag::InvalidateBuffer);

    i = 0;
    for (int pid = 0 ; pid < _Engine.s.largeparts.count ; pid++ ) {
        MxParticle *p  = &_Engine.s.largeparts.parts[pid];
        i += render_particle(pLargeData, i, p, &_Engine.s.largeparts);
    }
    
    assert(i == _Engine.s.nr_visible_large_parts);
    largeSphereInstanceBuffer.unmap();
    
    
    // render the cuboids.
    // get pointer to data, give me the damned bytes
    CuboidInstanceData* pCuboidData = (CuboidInstanceData*)(void*)cuboidInstanceBuffer.map(0,
            _Engine.s.nr_visible_cuboids * sizeof(CuboidInstanceData),
            GL::Buffer::MapFlag::Write|GL::Buffer::MapFlag::InvalidateBuffer);

    i = 0;
    for (int cid = 0 ; cid < _Engine.s.cuboids.size() ; cid++ ) {
        MxCuboid *c = &_Engine.s.cuboids[cid];
        i += render_cuboid(pCuboidData, i, c, _Engine.s.origin);
    }
    
    assert(i == _Engine.s.nr_visible_cuboids);
    cuboidInstanceBuffer.unmap();
    
    
    if(_Engine.nr_active_bonds > 0) {
        int vertexCount = _Engine.nr_active_bonds * 2;
        bondsMesh.setCount(vertexCount);
        
        bondsVertexBuffer.setData(
            {NULL, vertexCount * sizeof(BondsInstanceData)},
            GL::BufferUsage::DynamicDraw
        );
        
        // get pointer to data, give me the damned bytes
        BondsInstanceData* bondData = (BondsInstanceData*)(void*)bondsVertexBuffer.map(
           0,
           vertexCount * sizeof(BondsInstanceData),
           GL::Buffer::MapFlag::Write|GL::Buffer::MapFlag::InvalidateBuffer
        );
        
        int i = 0;
        Magnum::Vector3 *color;
        for(int j = 0; j < _Engine.nr_bonds; ++j) {
            MxBond *bond = &_Engine.bonds[j];
            i += render_bond(bondData, i, bond);
        }
        assert(i == 2 * _Engine.nr_active_bonds);
        bondsVertexBuffer.unmap();
        
        flatShader
        .setTransformationProjectionMatrix(camera->projectionMatrix() * camera->cameraMatrix() * modelViewMat)
        .draw(bondsMesh);
    }
    
    wireframeShader.setColor(Magnum::Color3{1., 1., 1.})
        .setTransformationProjectionMatrix(
             camera->projectionMatrix() *
             camera->cameraMatrix() *
             gridModelView)
        .draw(gridMesh);
    
    wireframeShader.setColor(Magnum::Color3::yellow())
        .draw(sceneBox);
    
    sphereShader
        .setProjectionMatrix(camera->projectionMatrix())
        .setTransformationMatrix(camera->cameraMatrix() * modelViewMat)
        .setNormalMatrix(camera->viewMatrix().normalMatrix());
    
    sphereShader.draw(sphereMesh);
    sphereShader.draw(largeSphereMesh);
    sphereShader.draw(cuboidMesh);

    angleRenderer.draw(camera, viewportSize, modelViewMat);
    dihedralRenderer.draw(camera, viewportSize, modelViewMat);
    arrowRenderer.draw(camera, viewportSize, modelViewMat);
    
    return *this;
}

void MxUniverseRenderer::setupCallbacks() {
    MX_NOTIMPLEMENTED_NORET
}

MxUniverseRenderer::~MxUniverseRenderer() {
    std::cout << MX_FUNCTION << std::endl;
}

void MxUniverseRenderer::onCursorMove(double xpos, double ypos)
{
    /*
    const Vector2i position(xpos, ypos);



    const Vector2 delta = 3.0f*Vector2{position - _prevMousePosition}/Vector2{window->framebufferSize()};
    _prevMousePosition = position;

    if(window->getMouseButtonState(MxGlfwWindow::MouseButtonLeft) == MxGlfwWindow::Press) {
        _objCamera->transformLocal(
            Matrix4::translation(_rotationPoint)*
            Matrix4::rotationX(-0.51_radf*delta.y())*
            Matrix4::rotationY(-0.51_radf*delta.x())*
            Matrix4::translation(-_rotationPoint));
    } else {
        const Vector3 p = unproject(position, _lastDepth);
        _objCamera->translateLocal(_translationPoint - p); // is Z always 0?
        _translationPoint = p;
    }

*/


}

MxVector3f MxUniverseRenderer::unproject(const MxVector2i& windowPosition, float depth) const {
    /* We have to take window size, not framebuffer size, since the position is
       in window coordinates and the two can be different on HiDPI systems */
    const MxVector2i viewSize = window->windowSize();
    const MxVector2i viewPosition = Vector2i{windowPosition.x(), viewSize.y() - windowPosition.y() - 1};
    const MxVector3f in{2.0f*MxVector2f{viewPosition}/MxVector2f{viewSize} - MxVector2f{1.0f}, depth*2.0f - 1.0f};

    return in;
}

void MxUniverseRenderer::onCursorEnter(int entered)
{
}



void MxUniverseRenderer::onRedraw()
{
}

void MxUniverseRenderer::onWindowMove(int x, int y)
{
}

void MxUniverseRenderer::onWindowSizeChange(int x, int y)
{
}

void MxUniverseRenderer::onFramebufferSizeChange(int x, int y)
{
}

void MxUniverseRenderer::draw() {
    
    Log(LOG_TRACE);
    
    window->framebuffer().clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    // Call arcball update in every frame. This will do nothing if the camera
    //   has not been changed. computes new transform.
    _arcball->updateTransformation();

    /* Trigger drawable object to update the particles to the GPU */
    setDirty();
    
    /* Draw particles */
    draw(_arcball, window->framebuffer().viewport().size());
}

/*
glfwSetFramebufferSizeCallback
#endif
(_window, [](GLFWwindow* const window, const int w, const int h) {
    auto& app = *static_cast<GlfwApplication*>(glfwGetWindowUserPointer(window));
    #ifdef MAGNUM_TARGET_GL
    ViewportEvent e{app.windowSize(), {w, h}, app.dpiScaling()};
    #else
    ViewportEvent e{{w, h}, app.dpiScaling()};
    #endif
    app.viewportEvent(e);
});

*/

void MxUniverseRenderer::viewportEvent(const int w, const int h) {
    /* Resize the main framebuffer */
    window->framebuffer().setViewport({{}, window->windowSize()});

    /* Recompute the camera's projection matrix */
    //_camera->setViewport(window->framebufferSize());

    //_arcball->reshape(event.windowSize(), event.framebufferSize());
}

void MxUniverseRenderer::onMouseButton(int button, int action, int mods)
{
}



void MxUniverseRenderer::viewportEvent(Platform::GlfwApplication::ViewportEvent& event) {
    window->framebuffer().setViewport({{}, event.framebufferSize()});

    _arcball->reshape(event.windowSize(), event.framebufferSize());

    // TODO: tell the shader
    //_shader.setViewportSize(Vector2{framebufferSize()});
}

static inline const bool cameraZoom(Magnum::Mechanica::ArcBallCamera *camera, const float &delta);

void MxUniverseRenderer::keyPressEvent(Platform::GlfwApplication::KeyEvent& event) {
    switch(event.key()) {
        case Platform::GlfwApplication::KeyEvent::Key::B: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->viewBottom(2 * sideLength);
                _arcball->translateToOrigin();
            }

            }
            break;

        case Platform::GlfwApplication::KeyEvent::Key::F: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->viewFront(2 * sideLength);
                _arcball->translateToOrigin();
            }

            }
            break;

        case Platform::GlfwApplication::KeyEvent::Key::K: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->viewBack(2 * sideLength);
                _arcball->translateToOrigin();
            }

            }
            break;

        case Platform::GlfwApplication::KeyEvent::Key::L: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->viewLeft(2 * sideLength);
                _arcball->translateToOrigin();
            }
            else {
                if(_arcball->lagging() > 0.0f) {
                    Debug{} << "Lagging disabled";
                    _arcball->setLagging(0.0f);
                } else {
                    Debug{} << "Lagging enabled";
                    _arcball->setLagging(0.85f);
                }
            }

            }
            break;

        case Platform::GlfwApplication::KeyEvent::Key::R: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->viewRight(2 * sideLength);
                _arcball->translateToOrigin();
            }
            else {
                _arcball->reset();
            }

            }
            break;
            
        case Platform::GlfwApplication::KeyEvent::Key::T: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->viewTop(2 * sideLength);
                _arcball->translateToOrigin();
            }

            }
            break;
            
        case Platform::GlfwApplication::KeyEvent::Key::Down: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Ctrl) {
                if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                    _arcball->translateDelta({0, 0, -_moveRate * sideLength});
                }
                else {
                    if(!cameraZoom(_arcball, - _zoomRate)) return;
                }
            }
            else if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->rotateDelta(&_spinRate, NULL, NULL);
            }
            else {
                _arcball->translateDelta({0, _moveRate * sideLength, 0});
            }
            
            }
            break;
            
        case Platform::GlfwApplication::KeyEvent::Key::Left: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Ctrl) {
                _arcball->rotateDelta(NULL, NULL, &_spinRate);
            }
            else if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                const float _ang(-_spinRate);
                _arcball->rotateDelta(NULL, &_ang, NULL);
            }
            else {
                _arcball->translateDelta({_moveRate * sideLength, 0, 0});
            }
            
            }
            break;
            
        case Platform::GlfwApplication::KeyEvent::Key::Right: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Ctrl) {
                const float _ang(-_spinRate);
                _arcball->rotateDelta(NULL, NULL, &_ang);
            }
            else if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                _arcball->rotateDelta(NULL, &_spinRate, NULL);
            }
            else {
                _arcball->translateDelta({-_moveRate * sideLength, 0, 0});
            }
            
            }
            break;
            
        case Platform::GlfwApplication::KeyEvent::Key::Up: {
            if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Ctrl) {
                if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                    _arcball->translateDelta({0, 0, _moveRate * sideLength});
                }
                else if(!cameraZoom(_arcball, _zoomRate)) return;
            }
            else if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
                const float _ang(-_spinRate);
                _arcball->rotateDelta(&_ang, NULL, NULL);
            }
            else {
                _arcball->translateDelta({0, -_moveRate * sideLength, 0});
            }
            
            }
            break;

        default: return;
    }

    event.setAccepted();
    window->redraw();
}

void MxUniverseRenderer::mousePressEvent(Platform::GlfwApplication::MouseEvent& event) {
    /* Enable mouse capture so the mouse can drag outside of the window */
    /** @todo replace once https://github.com/mosra/magnum/pull/419 is in */
    //SDL_CaptureMouse(SDL_TRUE);

    _arcball->initTransformation(event.position());

    event.setAccepted();
    window->redraw(); /* camera has changed, redraw! */

}

void MxUniverseRenderer::mouseReleaseEvent(Platform::GlfwApplication::MouseEvent& event) {

}

void MxUniverseRenderer::mouseMoveEvent(Platform::GlfwApplication::MouseMoveEvent& event) {
    if(!event.buttons()) return;

    if(event.modifiers() & Platform::GlfwApplication::MouseMoveEvent::Modifier::Shift) {
        _arcball->translate(event.position());
    }
    else if(event.modifiers() & Platform::GlfwApplication::MouseEvent::Modifier::Ctrl) {
        if(!cameraZoom(_arcball, - _zoomRate * event.relativePosition().y() * 0.1)) return;
    }
    else {
        _arcball->rotate(event.position());
    }

    event.setAccepted();
    window->redraw(); /* camera has changed, redraw! */
}

void MxUniverseRenderer::mouseScrollEvent(Platform::GlfwApplication::MouseScrollEvent& event) {
    if(!cameraZoom(_arcball, _zoomRate * event.offset().y())) return;

    event.setAccepted();
    window->redraw(); /* camera has changed, redraw! */
}


int MxUniverseRenderer::clipPlaneCount() const {
    return sphereShader.clipPlaneCount();
}

void MxUniverseRenderer::setClipPlaneEquation(unsigned id, const Magnum::Vector4& pe) {
    if(id > sphereShader.clipPlaneCount()) {
        mx_exp(std::invalid_argument("invalid id for clip plane"));
    }
    
    sphereShader.setclipPlaneEquation(id, pe);
    angleRenderer.setClipPlaneEquation(id, pe);
    dihedralRenderer.setClipPlaneEquation(id, pe);
    arrowRenderer.setClipPlaneEquation(id, pe);
    _clipPlanes[id] = pe;
}

const Magnum::Vector4& MxUniverseRenderer::getClipPlaneEquation(unsigned id) {
    return _clipPlanes[id];
}

const float MxUniverseRenderer::getZoomRate() {
    return _zoomRate;
}

void MxUniverseRenderer::setZoomRate(const float &zoomRate) {
    if(zoomRate <= 0.0 || zoomRate >= 1.0) {
        mx_exp(std::invalid_argument("invalid zoom rate (0, 1.0)"));
    }
    _zoomRate = zoomRate;
}

const float MxUniverseRenderer::getSpinRate() {
    return _spinRate;
}

void MxUniverseRenderer::setSpinRate(const float &spinRate) {
    _spinRate = spinRate;
}

const float MxUniverseRenderer::getMoveRate() {
    return _moveRate;
}

void MxUniverseRenderer::setMoveRate(const float &moveRate) {
    _moveRate = moveRate;
}

const bool cameraZoom(Magnum::Mechanica::ArcBallCamera *camera, const float &delta) 
{
    if(Math::abs(delta) < 1.0e-2f) return false;

    const float distance = camera->viewDistance() * delta;
    camera->zoom(distance);
    return true;
}

//void FluidSimApp::mouseScrollEvent(MouseScrollEvent& event) {
//    const Float delta = event.offset().y();
//    if(Math::abs(delta) < 1.0e-2f) {
//        return;
//    }
//
////    if(_imGuiContext.handleMouseScrollEvent(event)) {
////        /* Prevent scrolling the page */
////        event.setAccepted();
////        return;
////    }
//
//    const Float currentDepth = depthAt(event.position());
//    const Float depth = currentDepth == 1.0f ? _lastDepth : currentDepth;
//    const Vector3 p = unproject(event.position(), depth);
//    /* Update the rotation point only if we're not zooming against infinite
//       depth or if the original rotation point is not yet initialized */
//    if(currentDepth != 1.0f || _rotationPoint.isZero()) {
//        _rotationPoint = p;
//        _lastDepth = depth;
//    }
//
//    /* Move towards/backwards the rotation point in cam coords */
//    _objCamera->translateLocal(_rotationPoint * delta * 0.1f);
//}


//void MxUniverseRenderer::mousePressEvent(MouseEvent& event) {
//
//
//    if((event.button() != MouseEvent::Button::Left)
//       && (event.button() != MouseEvent::Button::Right)) {
//        return;
//    }
//
//    /* Update camera */
//    {
//        _prevMousePosition = event.position();
//        const Float currentDepth = depthAt(event.position());
//        const Float depth = currentDepth == 1.0f ? _lastDepth : currentDepth;
//        _translationPoint = unproject(event.position(), depth);
//
//        /* Update the rotation point only if we're not zooming against infinite
//           depth or if the original rotation point is not yet initialized */
//        if(currentDepth != 1.0f || _rotationPoint.isZero()) {
//            _rotationPoint = _translationPoint;
//            _lastDepth = depth;
//        }
//    }
//
//    _mousePressed = true;
//}
