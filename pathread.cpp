#include "pathread.hpp"

using namespace std;

int main() {
    

    std::vector<glm::mat4> cc = getPath("path.json");
    
    for(int j = 0; j < cc.size(); j++) {
	cout << glm::to_string(cc[j]) << endl;
    } 

    return 0;
}
