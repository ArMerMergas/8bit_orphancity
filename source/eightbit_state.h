#pragma once
#include <string>
#include <unordered_map>

struct EightbitState {
	float prootCutoff = 0.1;
	float prootGain = 1.5;
	float damp1 = 0.1;
	bool broadcastPackets = false;
	uint16_t port = 4000;
	std::string ip = "127.0.0.1";
	std::unordered_map<int, std::tuple<IVoiceCodec*, int>> afflictedPlayers;
};
