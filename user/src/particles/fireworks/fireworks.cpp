#include "particles/fireworks/fireworks.h"
// Firework manager

bool SetGameObjects_Instance1_Fireworks()
{
  CGameObject* firework_manager = gGameObjects["firework_manager"];
  if(!firework_manager)
  {
    firework_manager = gGameObjects.Add("firework_manager");
    firework_manager->Preserve();
    firework_manager->SetInputFunction(&Firework_Manager_Input);
    firework_manager->SetBehaviourFunction(&Firework_Manager_Behaviour);

    if(gData.GetInt("firework_explosion_particles") == __CSYSTEM_DATA_STORAGE_NOINT)
      gData.SetInt("firework_explosion_particles", 500);
  }

  firework_manager->AudioSource()->music = true;
  firework_manager->AudioSource()->loop = true;
  firework_manager->AudioSource()->everywhere = true;
  firework_manager->AudioSource()->start_playing = true;
  firework_manager->AudioSource()->SetSound("fireworks");
  firework_manager->AudioSource()->Bind();

  return true;
}

int number_of_fireworks = 0;
bool pressed = false;
vector<bool> exploded;
vector<CGameObject*> fireworks;
vector<int> fireworks_value;

void Firework_Manager_Input(CGameObject* gameObject)
{
  //if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE)
  vector<GO_InputClasses::CJoystick> joys = gUserInput.GetJoysticks();

  if(pressed and (joys.size() > 0 and joys[0].buttons.size() > 9 and joys[0].buttons[9].State() == Input::button_unpressed))
  {
    pressed = false;
  }
  if(gUserInput.jump() == Input::key_keydown or (joys.size() > 0 and joys[0].buttons.size() > 9 and joys[0].buttons[9].State() == Input::button_pressed and !pressed))
  {
    pressed = true; // joystick

    //std::string random_string = generate_random_alphanumeric_string(5);
    ostringstream oss;
    oss << number_of_fireworks;
    string value = oss.str();

    float timeout = gMath.random(1.f, 3.f);
    gSystem_Data_Storage.SetFloat("firework_timeout_"+value, timeout);
    gSystem_Data_Storage.SetFloat("firework_timer_"+value, gTime.GetTicks_s());

    CGameObject* firework = gGameObjects.Add("firework_"+value);
    CGameObject* explosion = gGameObjects.Add("firework_explosion_"+value);
    CGameObject* trail = gGameObjects.Add("firework_trail_"+value);

    firework->AddChild(explosion);
    firework->AddChild(trail);

    firework->Preserve(true);

    firework->Transform()->position = gMath.random_point(vector3f(50, 0, 0), vector3f(-25, 0, 0));

      // Explosion
    explosion->ParticleEmitter()->material_name = "sprite1";
    explosion->ParticleEmitter()->max_particles = gData.GetInt("firework_explosion_particles");
    explosion->ParticleEmitter()->particles_per_second = 0;
    explosion->ParticleEmitter()->angle_spread = 360;
    explosion->ParticleEmitter()->gravity(0, -5.f, 0);
    explosion->ParticleEmitter()->start_max_life_time = gMath.random(1.5f, 3.f);
    explosion->ParticleEmitter()->start_min_life_time = 1.f;

    GLfloat r = gMath.random(0.5, 1.f);
    GLfloat g = gMath.random(0.5, 1.f);
    GLfloat b = gMath.random(0.5, 1.f);

    explosion->ParticleEmitter()->start_min_color(r, g, b, 1.f);
    explosion->ParticleEmitter()->start_max_color(r, g, b, 1.f);

    r = gMath.random(0.f, 1.f);
    g = gMath.random(0.f, 1.f);
    b = gMath.random(0.f, 1.f);

    explosion->ParticleEmitter()->color_adder(r, g, b, 0.f);
    //explosion->particleEmitter()->color_adder(0, 0, 0, -0.1f);

    explosion->AudioSource()->SetSound("firework_explosion");
    explosion->AudioSource()->volume = 1.f;
    //explosion->AudioSource()->loop = true;
    explosion->AudioSource()->max_distance = 200.f;
    //explosion->AudioSource()->everywhere = false;
    //explosion->AudioSource()->Bind();

      // Trail
    trail->ParticleEmitter()->material_name = "smoke1";
    trail->ParticleEmitter()->max_particles = 500;
    trail->ParticleEmitter()->particles_per_second = 500;
    trail->ParticleEmitter()->angle_spread = 10;
    trail->ParticleEmitter()->direction(0, -1.f, 0);
    trail->ParticleEmitter()->gravity(0, 0.f, 0);

    trail->ParticleEmitter()->start_max_vel = 0.f;
    trail->ParticleEmitter()->start_min_vel = 0.f;

    trail->ParticleEmitter()->start_min_scale = 0.75f;
    trail->ParticleEmitter()->start_max_scale = 1.f;

    trail->ParticleEmitter()->start_min_color(1.f, 0.5f, 0.12f, 0.5f);
    trail->ParticleEmitter()->start_max_color(1.f, 0.5f, 0.12f, 0.8f);
    trail->ParticleEmitter()->min_color(0.12f, 0.12f, 0.12f, 0.f);
    trail->ParticleEmitter()->color_adder(-4.f, -2.f, 0.f, -1.f);

    trail->ParticleEmitter()->start_max_vel = 0.5f;
    trail->ParticleEmitter()->start_min_vel = 0.f;
    trail->ParticleEmitter()->start_max_life_time = 0.05f;
    trail->ParticleEmitter()->start_min_life_time = 0.85f;

    trail->ParticleEmitter()->Start();

    trail->AudioSource()->SetSound("firework_trail");
    trail->AudioSource()->volume = 0.6f;
    trail->AudioSource()->max_distance = 100.f;
    trail->AudioSource()->Bind();
    trail->AudioSource()->Play();

    fireworks_value.push_back(number_of_fireworks);
    exploded.push_back(false);
    fireworks.push_back(firework);
    number_of_fireworks++;

    //exploded.resize(number_of_fireworks);
    //exploded[exploded.size()-1] = false;
    //gGameObjects["camara_main"]->Camera()->SetTarget(firework->GetName());
  }
}

void Firework_Manager_Behaviour(CGameObject* gameObject)
{
  if(!fireworks.size())
    return;

  for(uint i = 0; i < fireworks.size(); i++)
  {
    ostringstream oss;
    oss << fireworks_value[i];
    string value = oss.str();

    CGameObject* current_firework = fireworks[i];

    float startTime = gSystem_Data_Storage.GetFloat("firework_timer_"+value);
    float timeout = gSystem_Data_Storage.GetFloat("firework_timeout_"+value);

    if((gTime.GetTicks_s() - startTime) < timeout/gTime.timeScale()) // 1.5 segundos.
    {
      current_firework->Transform()->Translate(0.f, 20.f*gTime.deltaTime_s(), 0.f);
    }
    else if(!exploded[i])
    {
      current_firework->GetChild(1)->ParticleEmitter()->Stop();
      current_firework->GetChild(0)->ParticleEmitter()->Start();
      current_firework->GetChild(0)->ParticleEmitter()->Stop();
      current_firework->GetChild(0)->AudioSource()->PlayOneShot();

      exploded[i] = true;
    }
    else if ((gTime.GetTicks_s() - startTime) > 5/gTime.timeScale())
    {
      // Clear float vars
      gSystem_Data_Storage.RemoveFloat("firework_timer_"+value);
      gSystem_Data_Storage.RemoveFloat("firework_timeout_"+value);

      // Clear vectors
      gGameObjects.Delete(current_firework->GetName(), true);

      exploded.erase(exploded.begin() + i);
      fireworks_value.erase(fireworks_value.begin() + i);
      fireworks.erase(fireworks.begin() + i);

      i--;


    }
    //Firework_Behaviour(gGameObjects["firework_"+value]);
  }
}

/*void Firework_Manager_Behaviour(CGameObject* gameObject)
{
  if(!number_of_fireworks)
    return;

  for(int i = 0; i < number_of_fireworks; i++)
  {
    ostringstream oss;
    oss << i;
    string value = oss.str();

    CGameObject* current_firework = gGameObjects["firework_"+value];

    float startTime = gSystem_Data_Storage.GetFloat("firework_timer_"+value);
    float timeout = gSystem_Data_Storage.GetFloat("firework_timeout_"+value);

    if((gTime.GetTicks_s() - startTime) < timeout/gTime.timeScale()) // 1.5 segundos.
    {
      current_firework->Transform()->Translate(0.f, 20.f*gTime.deltaTime_s(), 0.f);
    }
    else if(!exploded[i])
    {
      current_firework->GetChild("firework_trail_"+value)->ParticleEmitter()->Stop();
      current_firework->GetChild("firework_explosion_"+value)->ParticleEmitter()->Start();
      current_firework->GetChild("firework_explosion_"+value)->ParticleEmitter()->Stop();
      current_firework->GetChild("firework_explosion_"+value)->AudioSource()->PlayOneShot();

      //gMixer.PlaySound("leroy", current_firework);
      //gMixer.PlaySound("explosion", current_firework);

      exploded[i] = true;
    }
    //Firework_Behaviour(gGameObjects["firework_"+value]);
  }
}*/
