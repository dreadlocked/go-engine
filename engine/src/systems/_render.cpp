#include "_object.h"

#include "components/_component_camera.h"
#include "components/_component_transform.h"

#include "systems/_render.h"
#include "systems/_debug.h"
#include "systems/_data.h"
#include "systems/_manager.h"
#include "systems/_resource.h"

#include "engine/_engine.h"

CSystem_Render gSystem_Render;
CSystem_Render& gRender = gSystem_Render;

bool CSystem_Render::Init()
{
  CSystem::Init();

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);

  window = SDL_CreateWindow( gEngine.GetTitle().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gSystem_Data_Storage.GetInt("__RESOLUTION_WIDTH"), gSystem_Data_Storage.GetInt("__RESOLUTION_HEIGHT"), SDL_WINDOW_OPENGL);
  if(!window)
  {
    gSystem_Debug.error("From CSystem_Render: Could not create window: %s", SDL_GetError());
    gSystem_Debug.msg_box(ERROR_FATAL_INIT, "Could not create window. Check log.txt");
    return false;
  }

  GLcontext = SDL_GL_CreateContext(window);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0, 0, 0, 1);
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);

  glEnable( GL_BLEND );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0, 0, 0, 1.f);
  glClearDepth(1.0f);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glEnable(GL_SCISSOR_TEST);

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);

  glEnable(GL_COLOR_MATERIAL);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_ONE, GL_ONE);

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  GLenum error = glGetError();
  if(error != GL_NO_ERROR)
  {
    gSystem_Debug.error("From CSystem_Render: Could not init OpenGL: %s", gluErrorString(error));
    return false;
  }

  GLenum err = glewInit();
  if(err != GLEW_OK)
  {
    gSystem_Debug.error("From CSystem_Render: Could not init GLEW: %s", glewGetErrorString(err));
    return false;
  }

  if(!glewIsSupported("GL_ARB_multitexture"))
  {
    gSystem_Debug.error("From CSystem_Render: GLEW error: GL_ARB_multitexture NOT supported!");
    return false;
  }

  current_camera = -1;

  /*if(!glewIsSupported("GL_EXT_texture_env_combine"))
  {
    gSystem_Debug.error("From CSystem_Render: GLEW error: GL_EXT_texture_env_combine NOT supported!");
    return false;
  }*/

  return true;
}

void CSystem_Render::Close()
{
  CSystem::Close();
  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(window);
}

void CSystem_Render::SetMainCamera(CGameObject* camera)
{
  if(!camera)
    return;

  // Control de repeticiones de objetos
  vector<CGameObject*>::iterator it = camera_list.begin();
  camera_list.insert(it, camera);
  camera->camera()->ApplyChanges();
}

void CSystem_Render::SetMainCamera(const string& cam)
{
  CGameObject* camera = gSystem_GameObject_Manager[cam];
  if(!camera)
    return;

  // �?
  vector<CGameObject*>::iterator it = camera_list.begin();
  camera_list.insert(it, camera);
  camera->camera()->ApplyChanges();
}

void CSystem_Render::UnSetMainCamera(const string& camera)
{
  vector<CGameObject*>::iterator it = camera_list.begin();
  camera_list.erase(it);
}

void CSystem_Render::AddCamera(CGameObject* camera)
{
  if(camera)
  {
    camera_list.push_back(camera);
    camera->camera()->ApplyChanges();
  }
}

void CSystem_Render::AddCamera(const string& name)
{
  CGameObject* camera = gSystem_GameObject_Manager[name];
  if(camera)
  {
    camera_list.push_back(camera);
    camera->camera()->ApplyChanges();
  }
}

void CSystem_Render::AddCameraPrior(CGameObject* camera)
{
  if(camera_list.size() >= 1 && camera)
  {
    vector<CGameObject*>::iterator it = camera_list.begin();
    it++;
    camera_list.insert(it, camera);
    camera->camera()->ApplyChanges();
  }
}

void CSystem_Render::AddCameraPrior(const string& name)
{
  CGameObject* camera = gSystem_GameObject_Manager[name];
  if(camera_list.size() >= 1 && camera)
  {
    vector<CGameObject*>::iterator it = camera_list.begin();
    it++;
    camera_list.insert(it, camera);
    camera->camera()->ApplyChanges();
  }
}

void CSystem_Render::RemoveCamera(const string& camera)
{
  // ouh shit
  for(vector<CGameObject*>::iterator it = camera_list.begin(); it < camera_list.end(); it++)
  {
    if( (*it)->GetName() == camera )
      camera_list.erase(it);
  }
}

void CSystem_Render::OnLoop()
{
  GLenum error = glGetError();
  if(error != GL_NO_ERROR)
  {
    //gSystem_Debug.msg_box(debug::error, ERROR_RENDER, "From CSystem_Render: OpenGL error: %s", gluErrorString(error) );
    gSystem_Debug.console_error_msg("From CSystem_Render: OpenGL error: %s", gluErrorString(error));
  }
}

void CSystem_Render::OnRender()
{
  Clear();

  current_camera = 0;

  for(vector<CGameObject*>::iterator it = camera_list.begin(); it < camera_list.end(); it++)
  {
    // Disabled camera
    if(!(*it)->IsEnabled())
      continue;

    CComponent_Camera* cam = (*it)->GetComponent<CComponent_Camera>();

    cam->BeforeRender();
    cam->SetViewport();
    cam->SetUp();
    cam->Clear();

    // Draw Skybox
    glPushMatrix();
    DrawSkybox(cam);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    //glLoadIdentity();

    if(gSystem_Data_Storage.GetInt("__RENDER_TRANSFORM_GRID") )
      RenderGrid(gSystem_Data_Storage.GetInt("__RENDER_TRANSFORM_GRID_ROWS"), gSystem_Data_Storage.GetInt("__RENDER_TRANSFORM_GRID_COLS"));

    //glLoadIdentity();
    glPopMatrix();

	  for(map<string, CGameObject*>::iterator it2 = gSystem_GameObject_Manager.gameObjects.begin(); it2 != gSystem_GameObject_Manager.gameObjects.end(); it2++)
	  {
	    //glLoadIdentity();
	    glPushMatrix();

      glColor3f(1.f, 1.f, 1.f);
      glBindTexture(GL_TEXTURE_2D, 0);

      it2->second->transform()->ApplyTransform();
	    it2->second->OnRender();

	    glPopMatrix();
	  }

	  // Other renders
	  if(gSystem_Data_Storage.GetInt("__RENDER_TRANSFORM"))
	  {
	    GLboolean depth_state;
	    glGetBooleanv(GL_DEPTH_TEST, &depth_state);

	    if(depth_state) glDisable(GL_DEPTH_TEST);

	    for(map<string, CGameObject*>::iterator it2 = gSystem_GameObject_Manager.gameObjects.begin(); it2 != gSystem_GameObject_Manager.gameObjects.end(); it2++)
	    {
	       glPushMatrix();

	       it2->second->transform()->ApplyTransform();
	       it2->second->transform()->OnRender();

	       glPopMatrix();
	    }

	    if(depth_state) glEnable(GL_DEPTH_TEST);
	  }

    cam->AfterRender();
    current_camera++;

  }
  current_camera = -1;
}

void CSystem_Render::RenderGrid(int rows, int cols)
{
  glBindTexture(GL_TEXTURE_2D, 0);

  GLfloat cols_size = gSystem_Data_Storage.GetFloat("__RENDER_TRANSFORM_GRID_COLS_SIZE");
  GLfloat rows_size = gSystem_Data_Storage.GetFloat("__RENDER_TRANSFORM_GRID_ROWS_SIZE");

  glTranslatef((-cols*cols_size)/2.f, 0.f, -(rows*rows_size)/2.f);

  glBegin(GL_LINES);
    // Horizontal lines.

    for (int i = 0; i <= rows; i++)
    {
      if(i % 5) glColor3f(0.5f, 0.5f, 0.5f);
      else glColor3f(1.f, 1.f, 1.f);

      glVertex3f(0, 0, i*cols_size);
      glVertex3f(cols*cols_size, 0, i*cols_size);
    }
    // Vertical lines.
    for (int i = 0; i <= cols; i++)
    {
      if(i % 5) glColor3f(0.5f, 0.5f, 0.5f);
      else glColor3f(1.f, 1.f, 1.f);

      glVertex3f(i*rows_size, 0, 0);
      glVertex3f(i*rows_size, 0, rows*rows_size);
    }
  glEnd();
}


bool CSystem_Render::DrawSkybox(CComponent_Camera* cam)
{
  if(cam->skybox_texture == "")
    return false;

  // Idea sencilla: dibujamos un cubo de tama�o 1x1 sin depth_test justo donde esta la c�mara.
  // Todo lo dem�s se dibujar� encima, por lo que crear� un efecto de "tama�o inmenso".
  // http://content.gpwiki.org/index.php/Sky_Box
  glPushMatrix();

  //glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glDisable(GL_DEPTH_TEST);

  glColor3f(1.f, 1.f, 1.f);
  glBindTexture(GL_TEXTURE_2D, gSystem_Resources.GetTexture(cam->skybox_texture )->GetID());

  vector3f position = cam->gameObject->transform()->Position();
  glTranslatef(position.x, position.y, position.z);

  float r = 1.005f; // If you have border issues change this to 1.005f

  glBegin(GL_QUADS);
      // Top
    glTexCoord2f( 512/2048.f, 1536/1536.f); glVertex3f( r,  1.0f, -r);
    glTexCoord2f(1024/2048.f, 1536/1536.f); glVertex3f(-r,  1.0f, -r);
    glTexCoord2f(1024/2048.f, 1024/1536.f); glVertex3f(-r,  1.0f,  r);
    glTexCoord2f( 512/2048.f, 1024/1536.f); glVertex3f( r,  1.0f,  r);

      // Bottom
    glTexCoord2f( 512/2048.f, 512/1536.f);  glVertex3f( r, -1.0f,  r);
    glTexCoord2f(1024/2048.f, 512/1536.f);  glVertex3f(-r, -1.0f,  r);
    glTexCoord2f(1024/2048.f,   0/1536.f);  glVertex3f(-r, -1.0f, -r);
    glTexCoord2f( 512/2048.f,   0/1536.f);  glVertex3f( r, -1.0f, -r);

      // Left
    glTexCoord2f(   0/2048.f, 1024/1536.f); glVertex3f( 1.0f,  r, -r);
    glTexCoord2f( 512/2048.f, 1024/1536.f); glVertex3f( 1.0f,  r,  r);
    glTexCoord2f( 512/2048.f,  512/1536.f); glVertex3f( 1.0f, -r,  r);
    glTexCoord2f(   0/2048.f,  512/1536.f); glVertex3f( 1.0f, -r, -r);

      // Right
    glTexCoord2f(1024/2048.f, 1024/1536.f); glVertex3f(-1.0f,  r,  r);
    glTexCoord2f(1536/2048.f, 1024/1536.f); glVertex3f(-1.0f,  r, -r);
    glTexCoord2f(1536/2048.f,  512/1536.f); glVertex3f(-1.0f, -r, -r);
    glTexCoord2f(1024/2048.f,  512/1536.f); glVertex3f(-1.0f, -r,  r);

      // Front
    glTexCoord2f( 512/2048.f, 1024/1536.f); glVertex3f( r,  r,  1.0f);
    glTexCoord2f(1024/2048.f, 1024/1536.f); glVertex3f(-r,  r,  1.0f);
    glTexCoord2f(1024/2048.f, 512/1536.f);  glVertex3f(-r, -r,  1.0f);
    glTexCoord2f( 512/2048.f, 512/1536.f);  glVertex3f( r, -r,  1.0f);

      // Back
    glTexCoord2f(1536/2048.f, 1024/1536.f); glVertex3f(-r,  r, -1.0f);
    glTexCoord2f(2048/2048.f, 1024/1536.f); glVertex3f( r,  r, -1.0f);
    glTexCoord2f(2048/2048.f, 512/1536.f);  glVertex3f( r, -r, -1.0f);
    glTexCoord2f(1536/2048.f, 512/1536.f);  glVertex3f(-r, -r, -1.0f);
    glEnd();

  /*glBegin(GL_QUADS);
    // Top
    glTexCoord2f( 512/2048.f, 0/1536.f);   glVertex3f( r, -r, 1.0f);
    glTexCoord2f(1024/2048.f, 0/1536.f);   glVertex3f( r,  r, 1.0f);
    glTexCoord2f(1024/2048.f, 512/1536.f); glVertex3f(-r,  r, 1.0f);
    glTexCoord2f( 512/2048.f, 512/1536.f); glVertex3f(-r, -r, 1.0f);

    // Bottom
    glTexCoord2f( 512/2048.f, 1024/1536.f); glVertex3f( r, -r, -1.0f);
    glTexCoord2f(1024/2048.f, 1024/1536.f); glVertex3f( r,  r, -1.0f);
    glTexCoord2f(1024/2048.f, 1536/1536.f); glVertex3f(-r,  r, -1.0f);
    glTexCoord2f( 512/2048.f, 1536/1536.f); glVertex3f(-r, -r, -1.0f);

    // Front
    glTexCoord2f( 512/2048.f, 512/1536.f);  glVertex3f(-r, 1.0f,-r);
    glTexCoord2f(1024/2048.f, 512/1536.f);  glVertex3f(-r, 1.0f, r);
    glTexCoord2f(1024/2048.f, 1024/1536.f); glVertex3f( r, 1.0f, r);
    glTexCoord2f( 512/2048.f, 1024/1536.f); glVertex3f( r, 1.0f,-r);

    // Back
    glTexCoord2f(1536/2048.f, 512/1536.f);  glVertex3f(-r,-1.0f,-r);
    glTexCoord2f(2048/2048.f, 512/1536.f);  glVertex3f(-r,-1.0f, r);
    glTexCoord2f(2048/2048.f, 1024/1536.f); glVertex3f( r,-1.0f, r);
    glTexCoord2f(1536/2048.f, 1024/1536.f); glVertex3f( r,-1.0f,-r);

    // Left
    glTexCoord2f(1024/2048.f, 512/1536.f);  glVertex3f(-1.0f, -r, r);
    glTexCoord2f(1536/2048.f, 512/1536.f);  glVertex3f(-1.0f,  r, r);
    glTexCoord2f(1536/2048.f, 1024/1536.f); glVertex3f(-1.0f,  r,-r);
    glTexCoord2f(1024/2048.f, 1024/1536.f); glVertex3f(-1.0f, -r,-r);

    // Right
    glTexCoord2f(  0/2048.f, 512/1536.f);  glVertex3f(1.0f, -r, r);
    glTexCoord2f(512/2048.f, 512/1536.f);  glVertex3f(1.0f,  r, r);
    glTexCoord2f(512/2048.f, 1024/1536.f); glVertex3f(1.0f,  r,-r);
    glTexCoord2f(  0/2048.f, 1024/1536.f); glVertex3f(1.0f, -r,-r);
  glEnd();*/

  //glPopAttrib();
  glClear(GL_DEPTH_BUFFER_BIT);
  glPopMatrix();

  return true;
}

void CSystem_Render::Clear()
{
  glScissor(0, 0, gSystem_Data_Storage.GetInt("__RESOLUTION_WIDTH"), gSystem_Data_Storage.GetInt("__RESOLUTION_HEIGHT"));

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



