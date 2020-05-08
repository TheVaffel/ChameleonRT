#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using json = nlohmann::json;

struct CameraCheckpoint {
    glm::vec3 point;
    glm::vec3 dir;
  glm::vec3 light_pos;
  float light_size;
    int t;
  CameraCheckpoint() {
    light_size = -1;
  }
};

struct LightState {
  glm::vec3 pos;
  float size;
  int t;
};

struct TimeStampState {
  glm::mat4 cam_mat;
  glm::vec3 light_pos;
  float light_size;
  int t;
};

CameraCheckpoint cpFromObj(const json& j) {
    CameraCheckpoint cc;
    cc.point = glm::vec3(j["x"], j["y"], j["z"]);
    cc.dir = glm::vec3(j["dirx"], j["diry"], j["dirz"]);
    if(j.count("light_x") > 0 &&
       j.count("light_y") > 0 &&
       j.count("light_z") > 0) {
      cc.light_pos = glm::vec3(j["light_x"], j["light_y"], j["light_z"]);
      if(j.count("light_size") > 0) {
	cc.light_size = j["light_size"];
      } else {
	cc.light_size = 2.0;
      }
    }
    cc.t = int(j["t"]);
    return cc;
}

void getInterpolatedLight(TimeStampState& tts, const CameraCheckpoint& cc1,
			  const CameraCheckpoint& cc2,
			  int t) {
  float coeff = float(t - cc1.t) / float(cc2.t - cc1.t);
  tts.light_pos = coeff * cc2.light_pos + (1 - coeff) * cc1.light_pos;
  tts.light_size = coeff * cc2.light_size + (1 - coeff) * cc1.light_size;
}

glm::mat4 getInterpolatedView(const CameraCheckpoint& cc1,
			      const CameraCheckpoint& cc2,
			      int t) {
    float coeff = float(t - cc1.t) / float(cc2.t - cc1.t);
    CameraCheckpoint cc;
    cc.point = coeff * cc2.point + (1 - coeff) * cc1.point;
    cc.dir = -glm::normalize(coeff * cc2.dir + (1 - coeff) * cc1.dir);

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 x_axis = glm::normalize(glm::cross(cc.dir, up));
    glm::vec3 y_axis = glm::normalize(glm::cross(x_axis, cc.dir));

    glm::mat3 rotation = glm::transpose(glm::mat3(x_axis, y_axis, cc.dir));
    glm::mat4 transform = glm::mat4(rotation) * glm::translate(cc.point) ;
    return transform;
}

std::vector<TimeStampState> getTimeStampStates(const std::string& str) {
    std::ifstream fs(str);
    /* std::stringstream buffer;
    buffer << fs.rdbuf();
    std::string ss = buffer.str();

    json list = json::parse(ss); */
    
    json list;
    fs >> list;

    if(!list.is_array()) {
	std::cerr << "JSON object was not a list!" << std::endl;
	std::exit(0);
    }
    
    std::vector<CameraCheckpoint> cps;
    std::vector<LightState> light_states;
    for (json::iterator it = list.begin(); it != list.end(); ++it) {
	cps.push_back(cpFromObj(*it));
	if(cps[cps.size() - 1].light_size >= 0) {
	  LightState ls;
	  ls.pos = cps[cps.size() - 1].light_pos;
	  ls.size = cps[cps.size() - 1].light_size;
	  ls.t = cps[cps.size() - 1].t;
	  light_states.push_back(ls);
	}
    }

    int last_light = -1;
    for(unsigned int i = 0; i < cps.size(); i++) {
      if(cps[i].light_size >= 0) {
	if(last_light  == -1) {
	  for(unsigned int j = 0; j < i; j++) {
	    cps[j].light_size = cps[i].light_size;
	    cps[j].light_pos = cps[i].light_pos;
	  }
	} else {
	  for(unsigned int j = last_light + 1; j < i; j++) {
	    float coeff = float(cps[j].t - cps[last_light].t) / float(cps[i].t - cps[last_light].t);
	    cps[j].light_size = coeff * cps[i].light_size + (1 - coeff) * cps[last_light].light_size;
	    cps[j].light_pos = coeff * cps[i].light_pos + (1 - coeff) * cps[last_light].light_pos;
	  }
	}
	last_light = i;
      }
    }

    if(last_light != -1 && last_light != int(cps.size()) - 1) {
      for(unsigned int i = last_light + 1; i < cps.size(); i++) {
	cps[i].light_size = cps[last_light].light_size;
	cps[i].light_pos = cps[last_light].light_pos;
      }
    }

    std::vector<TimeStampState> states;
    int t = cps[0].t;
    size_t current_cp = 0;
    while(current_cp < cps.size() - 1) {
      TimeStampState tts;
      tts.cam_mat = getInterpolatedView(cps[current_cp], cps[current_cp + 1], t);
      getInterpolatedLight(tts, cps[current_cp], cps[current_cp + 1], t);
	states.push_back(tts);
	t++;
	if (t >= cps[current_cp + 1].t) {
	    current_cp++;
	}
    }

    TimeStampState tts;
    tts.cam_mat = getInterpolatedView(cps[current_cp - 1], cps[current_cp], t);
    getInterpolatedLight(tts, cps[current_cp - 1], cps[current_cp], t);
    states.push_back(tts);

    return states;
}
