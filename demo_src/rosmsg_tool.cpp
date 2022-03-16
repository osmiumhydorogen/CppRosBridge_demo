#include <jansson.h>
#include <vector>

#include "rosmsg_tool.hpp"

void F32Array2Vec(json_t *msg_arr, std::vector<float> *vec)
{
	json_t *data_json = json_object_get(msg_arr, "data");
	int n_data = json_array_size(data_json);
	vec->resize(n_data);

	int i;
	json_t *val;
	json_array_foreach(data_json, i, val)
	{
		(*vec)[i] = json_real_value(val);
	}
	//json_decref(data_json);
	json_decref(val);
}

json_t *buildF32MultiArr(const std::vector<float> &vec)
{
	json_t *data_json = json_array();
	json_t *msg = json_object();
	int sz = vec.size();

	for(auto &&val : vec)
	{
		json_array_append_new(data_json, json_real(val));
	}
	json_object_set_new(msg, "data", data_json);
	json_object_set_new(msg, "layout",
		json_pack("{s:[{s:i,s:i,s:s}],s:i}", "dim", "stride", sz, "size", sz, "label", "x", "data_offset", 0)
	);
	return msg;
}
