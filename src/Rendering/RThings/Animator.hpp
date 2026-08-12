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
	std::vector<ENGINE::ImageView *>    imagesFrames;
	std::vector<std::vector<glm::vec4>> imagesFramesCPU;
	AnimatorInfo                        animatorInfo;
	int                                 currentFrameSpacing = 0;
	int                                 frameSpacing        = 0;
	bool                                stop                = false;

	void LoadFrames(std::string &filePath, int frameSpacing)
	{
		std::filesystem::path path(filePath);
		int                   i = 0;
		std::vector<ENGINE::ImageShipper*> shippers;
		imagesFrames.clear();
		imagesFramesCPU.clear();
		this->frameSpacing        = std::max(1, frameSpacing);
		this->currentFrameSpacing = 0;
		
		shippers.reserve(1000);
		for (auto &file : std::filesystem::directory_iterator(path))
		{
			if (!is_regular_file(file))
			{
				continue;
			}
			// if (path.extension() != "png" || )
			ENGINE::ImageShipper *imageShipper = ENGINE::ResourcesManager::GetInstance()->GetShipper(
			    "Frame_" + std::to_string(i), file.path().string(), 1, 1, ENGINE::g_ShipperFormat, ENGINE::GRAPHICS_READ);
			shippers.push_back(imageShipper);
			imagesFrames.push_back(imageShipper->imageView.get());
			i++;
		}
		this->animatorInfo.frameCount   = static_cast<int>(imagesFrames.size());
		this->animatorInfo.currentFrame = 0;
		this->animatorInfo.interpVal    = 0.0f;
	}
	void LoadFramesCPU(std::string &folderPath, bool flipY)
	{
		std::filesystem::path path(folderPath);
		std::vector<std::filesystem::path> framePaths;
		imagesFramesCPU.clear();
		currentFrameSpacing = 0;
		animatorInfo        = {};

		std::error_code directoryError;
		for (auto &file : std::filesystem::directory_iterator(path, directoryError))
		{
			if (directoryError)
			{
				break;
			}
			if (file.is_regular_file())
			{
				framePaths.push_back(file.path());
			}
		}
		std::sort(framePaths.begin(), framePaths.end());

		for (const auto &framePath : framePaths)
		{
			ENGINE::ImageShipper imageShipper;
			if (!imageShipper.SetDataFromPath(framePath.string()))
			{
				continue;
			}

			const glm::uvec2 frameSize(
			    static_cast<uint32_t>(imageShipper.imageSize.x),
			    static_cast<uint32_t>(imageShipper.imageSize.y));
			if (!imagesFramesCPU.empty() && frameSize != animatorInfo.spriteSizePx)
			{
				SYSTEMS::Logger::GetInstance()->Log(
				    "Skipping animation frame with a different size: " + framePath.string(),
				    SYSTEMS::LogLevel::L_WARN);
				imageShipper.Clear();
				continue;
			}
			if (imagesFramesCPU.empty())
			{
				animatorInfo.spriteSizePx = frameSize;
			}
			imageShipper.BuildCPUImage(".", 0, flipY);
			imagesFramesCPU.emplace_back(imageShipper.ShipImageCPU());
		}
		animatorInfo.frameCount   = static_cast<int>(imagesFramesCPU.size());
		animatorInfo.currentFrame = 0;
		animatorInfo.interpVal    = 0.0f;
	}
	
	void ClearCPU()
	{
		imagesFramesCPU.clear();
		animatorInfo        = {};
		currentFrameSpacing = 0;
		stop                = true;
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

	
	std::vector<glm::vec4> *UseCPUFrame()
	{
		if (imagesFramesCPU.empty())
		{
			return nullptr;
		}

		const int frameCount = static_cast<int>(imagesFramesCPU.size());
		animatorInfo.frameCount = frameCount;
		animatorInfo.currentFrame = std::max(0, animatorInfo.currentFrame) % frameCount;
		const int index = animatorInfo.currentFrame;
		if (stop)
		{
			return &imagesFramesCPU.at(index);
		}
		const int safeFrameSpacing   = std::max(1, frameSpacing);
		this->animatorInfo.interpVal = static_cast<float>(currentFrameSpacing) / static_cast<float>(safeFrameSpacing);
		currentFrameSpacing          = (currentFrameSpacing + 1) % safeFrameSpacing;
		if (currentFrameSpacing == 0)
		{
			animatorInfo.currentFrame = (animatorInfo.currentFrame + 1) % frameCount;
		}
		return &imagesFramesCPU.at(index);
	}
	ENGINE::ImageView *UseFrame()
	{
		if (imagesFrames.empty())
		{
			return nullptr;
		}

		const int frameCount = static_cast<int>(imagesFrames.size());
		animatorInfo.frameCount = frameCount;
		animatorInfo.currentFrame = std::max(0, animatorInfo.currentFrame) % frameCount;
		const int index = animatorInfo.currentFrame;
		if (stop)
		{
			return imagesFrames.at(index);
		}
		const int safeFrameSpacing   = std::max(1, frameSpacing);
		this->animatorInfo.interpVal = static_cast<float>(currentFrameSpacing) / static_cast<float>(safeFrameSpacing);
		currentFrameSpacing          = (currentFrameSpacing + 1) % safeFrameSpacing;
		if (currentFrameSpacing == 0)
		{
			animatorInfo.currentFrame = (animatorInfo.currentFrame + 1) % frameCount;
		}
		return imagesFrames.at(index);
	}
};
}        // namespace Rendering
#endif        // ANIMATOR_HPP
