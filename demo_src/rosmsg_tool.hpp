#include <jansson.h>
#include <vector>

void F32Array2Vec(json_t *msg_arr, std::vector<float> *vec);
json_t *buildF32MultiArr(const std::vector<float> &vec);
