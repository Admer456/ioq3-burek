#pragma once

#include <vector>

class GameMusic final
{
public:
	static constexpr uint16_t NoSong = 65535U;

	class Song final
	{
	public:
		Song( const char* songLabel, const float& songLength );
		Song() = default;
		~Song() = default;

		void Play() const;

		char label[32]{};
		float length{ 0.0f };
	};

	static constexpr const char* ActionLabels[] =
	{
		"action1", "action2", "action3"
	};

public:
	static void Initialise( const char* musicFile );
	static void SetNextLabel( const char* label, bool forceStart = false );
	static void Update( const float& time );

	static bool IsAmbientPlaying();

	//static void ToCustom( const char* customLabel );
	static void	ToAction();
	static void ToAmbient();

private:
	static inline std::vector<GameMusic::Song> songs;

	static inline uint16_t currentSong{ NoSong };
	static inline uint16_t nextSong{ NoSong };
		   
	static inline float currentLabelStart{ 0.0f }; // the beginning of the current label
	static inline float nextLabel{ 0.0f }; // when the next one will play

	static inline bool noSongs{ true };
	static inline bool isAction{ false };
};
