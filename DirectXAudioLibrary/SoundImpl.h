#pragma once
//#include"SoundBuffer.h"
#include"Deleter.hxx"
#include<unordered_map>
#include<memory>

class SoundBuffer;
using MSSourceVoice = std::unique_ptr<IXAudio2SourceVoice, decltype(&VoiceDeleter)>;
using MSXAudio2 = std::shared_ptr<IXAudio2>;
class SoundDevice;
//データベース
class SoundImpl {
public:
	explicit SoundImpl(MSXAudio2&aXAudio);
private:
	friend class SoundDevice;
	//事前登録
	void PreLoad(const std::string&aFileName);
	//登録してボイス作成
	MSSourceVoice Load(const std::string & aFileName);
	std::unordered_map<std::string, SoundBuffer>mBuffer;

	MSXAudio2 mXAudio;//外部で作成
};

