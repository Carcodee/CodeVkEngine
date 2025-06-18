//

// Created by carlo on 2025-01-01.
//

#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP

namespace Rendering
{
struct AnimatorInfo
{
	glm::uvec2 spriteSizePx = glm::uvec2(0);
	int        rows         = -1;
	int        cols         = -1;
	int        currentFrame = -1;
	int        frameCount   = -1;
	float      interpVal    = -1.0f;
	bool       isAtlas      = false;
};
struct Animator2D
{
	std::vector<ENGINE::ImageView *> imagesFrames;
	AnimatorInfo                     animatorInfo;
	int                              currentFrameSpacing = 0;
	int                              frameSpacing        = 0;
	bool                             stop                = false;

	void LoadFrames(std::string &filePath, int frameSpacing)
	{
		std::filesystem::path path(filePath);
		int                   i = 0;
		for (auto &file : std::filesystem::directory_iterator(path))
		{
			if (!is_regular_file(file))
			{
				continue;
			}
			// if (path.extension() != "png" || )
			ENGINE::ImageView *imageView = ENGINE::ResourcesManager::GetInstance()->GetShipper(
			                                                                          "Frame_" + std::to_string(i), file.path().string(), 1, 1, ENGINE::g_ShipperFormat,
			                                                                          ENGINE::GRAPHICS_READ)
			                                   ->imageView.get();
			imagesFrames.push_back(imageView);
			i++;
		}
		this->animatorInfo.currentFrame = 0;
	}

	void LoadAtlas(std::string path, int frameSpacing, AnimatorInfo animatorInfo)
	{
		if (!std::filesystem::is_regular_file(path))
		{
			SYSTEMS::Logger::GetInstance()->Log("Invalid file type when loading atlas");
			return;
		}
		this->frameSpacing = frameSpacing;
		this->animatorInfo = animatorInfo;

		ENGINE::ImageView *imageView = ENGINE::ResourcesManager::GetInstance()->GetShipper(
		                                                                          "Atlas_", path, 1, 1, ENGINE::g_ShipperFormat,
		                                                                          ENGINE::GRAPHICS_READ)
		                                   ->imageView.get();
		imagesFrames.push_back(imageView);
		this->animatorInfo.frameCount   = animatorInfo.cols * animatorInfo.rows;
		this->animatorInfo.currentFrame = 0;
		this->animatorInfo.interpVal    = 0.0f;
	}

	ENGINE::ImageView *UseFrame()
	{
		int index = animatorInfo.currentFrame;
		if (stop)
		{
			return imagesFrames.at(index % imagesFrames.size());
		}
		this->animatorInfo.interpVal = (float) currentFrameSpacing / (float) frameSpacing;
		currentFrameSpacing          = (currentFrameSpacing + 1) % frameSpacing;
		if (currentFrameSpacing == 0)
		{
			animatorInfo.currentFrame++;
			animatorInfo.currentFrame = animatorInfo.currentFrame % animatorInfo.frameCount;
		}
		return imagesFrames.at(index % imagesFrames.size());
	}
};
}        // namespace Rendering
#endif        // ANIMATOR_HPP
