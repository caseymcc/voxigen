#ifndef _voxigen_faces_h_
#define _voxigen_faces_h_

#include <glm/glm.hpp>
#include <array>

namespace voxigen
{

namespace Face
{
const size_t xNeg=0;
const size_t xPos=1;
const size_t yNeg=2;
const size_t yPos=3;
const size_t zNeg=4;
const size_t zPos=5;

const size_t left=0;
const size_t right=1;
const size_t front=2;
const size_t back=3;
const size_t bottom=4;
const size_t top=5;
}

const std::array<std::array<glm::ivec3, 4>, 6> faces=
{{
    {{
        {0, 1, 0},
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 1}
    }},
    {{
        {1, 0, 0},
        {1, 1, 0},
        {1, 1, 1},
        {1, 0, 1}
    }},
    {{
        {0, 0, 0},
        {1, 0, 0},
        {1, 0, 1},
        {0, 0, 1}
    }},
    {{
        {1, 1, 0},
        {0, 1, 0},
        {0, 1, 1},
        {1, 1, 1}
    }},
    {{
        {1, 0, 0},
        {0, 0, 0},
        {0, 1, 0},
        {1, 1, 0}
    }},
    {{
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 1, 1}
    }}
}};

const std::array<std::array<glm::ivec3, 5>, 6> facesWithNormal=
{{
    {{
        {0, 1, 0},
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 1}, 
        {-1, 0, 0}
    }},
    {{
        {1, 0, 0},
        {1, 1, 0},
        {1, 1, 1},
        {1, 0, 1},
        {1, 0, 0}
    }},
    {{
        {0, 0, 0},
        {1, 0, 0},
        {1, 0, 1},
        {0, 0, 1},
        {0, -1, 0}
    }},
    {{
        {1, 1, 0},
        {0, 1, 0},
        {0, 1, 1},
        {1, 1, 1},
        {0, 1, 0}
    }},
    {{
        {1, 0, 0},
        {0, 0, 0},
        {0, 1, 0},
        {1, 1, 0},
        {0, 0, -1}
    }},
    {{
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 1, 1},
        {0, 0, 1}
    }}
}};

} //namespace voxigen

#endif //_voxigen_faces_h_