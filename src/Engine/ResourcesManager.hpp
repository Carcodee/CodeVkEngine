//





// Created by carlo on 2024-11-22.
//




#ifndef RESOURCESMANAGER_HPP
#define RESOURCESMANAGER_HPP

#define BASE_SIZE 10000

namespace ENGINE
{
    class ResourcesManager : SYSTEMS::Subject
    {
    public:
        enum ResState
        {
            VALID,
            INVALID
        };

        struct BufferUpdateInfo
        {
            ResState state;
            size_t size;
            void* data;
        };
        struct ImgCreateConfigs
        {
            int baseMipLevel = 0;
            int baseArrayLayer = 0; 
            bool addToStorageClear = false;           
        };

        struct ImgUpdateInfo
        {
            ResState bufferState;
            std::string path;
            uint32_t arrayLayersCount;
            uint32_t mipsCount;
            vk::Format format;
            LayoutPatterns dstPattern;
            std::string name;
            int32_t id;
        };
        
        struct BufferBatchInfo
        {
            std::string name;
            size_t size;
            void* data;
            vk::BufferUsageFlags bufferUsageFlags;
            vk::MemoryPropertyFlags memPropertyFlags;
        };

        struct ShaderBatchInfo
        {
            std::string path;
            ShaderStage stage;
            int32_t id;
            
        };

        struct DsetsInfo
        {
            vk::DescriptorSet dset;
            int32_t id;
        };

        ImageShipper* GetShipper(std::string name, std::string path, uint32_t arrayLayersCount, uint32_t mipsCount,
                                 vk::Format format,
                                 LayoutPatterns dstPattern)
        {
            assert(core!= nullptr &&"core must be set");
            ImageShipper* imageShipper;
            int32_t id = (int32_t)imageShippers.size();
            if (imagesShippersNames.contains(name))
            {
                imageShipper = imageShippers.at(imagesShippersNames.at(name)).get();
            }
            else
            {
                imagesShippersNames.try_emplace(name, id);
                imageShippers.emplace_back(std::make_unique<ImageShipper>());
                imagesUpdateInfos.emplace_back(ImgUpdateInfo{
                     VALID, path, arrayLayersCount, mipsCount, format, dstPattern, name, id
                });
                imageShipper = GetShipperFromName(name);
            }
            if (imageShipper->image == nullptr)
            {
                imageShipper->SetDataFromPath(path);
                imageShipper->BuildImage(core, shipperSampler, arrayLayersCount, mipsCount, format, dstPattern, name,
                                         id);
            }
            return imageShipper;
        }

        ImageShipper* GetShipper(std::string name, void* data, int width, int height, vk::DeviceSize size,
                                 uint32_t arrayLayersCount, uint32_t mipsCount, vk::Format format,
                                 LayoutPatterns dstPattern)
        {
            assert(core!= nullptr &&"core must be set");
            ImageShipper* imageShipper;
            int32_t id = (int32_t)imageShippers.size();
            if (imagesShippersNames.contains(name))
            {
                imageShipper = imageShippers.at(imagesShippersNames.at(name)).get();
            }
            else
            {
                imagesShippersNames.try_emplace(name, id);
                imageShippers.emplace_back(std::make_unique<ImageShipper>());
                imagesUpdateInfos.emplace_back(ImgUpdateInfo{
                    VALID, "", arrayLayersCount, mipsCount, format, dstPattern,  name, id
                });
                imageShipper = GetShipperFromName(name);
            }
            if (imageShipper->image == nullptr)
            {
                imageShipper->SetDataRaw(data, width, height, size);
                imageShipper->BuildImage(core, shipperSampler, arrayLayersCount, mipsCount, format, dstPattern, name,
                                         id);
            }
            return imageShipper;
        }

        ImageShipper* BatchShipper(std::string name, std::string path, uint32_t arrayLayersCount, uint32_t mipsCount,
                                   vk::Format format,
                                   LayoutPatterns dstPattern)
        {
            if (imagesShippersNames.contains(name))
            {
                SYSTEMS::Logger::GetInstance()->Log("Using texture that already exist: " + name,
                                                    SYSTEMS::LogLevel::L_INFO);
                ImageShipper* shipper = GetShipperFromName(name);
                return shipper;
            }
            int id = (int32_t)imageShippers.size();
            imagesShippersNames.try_emplace(name, id);
            imageShippers.emplace_back(std::make_unique<ImageShipper>());
            imagesUpdateInfos.emplace_back(ImgUpdateInfo{
                INVALID, path, arrayLayersCount, mipsCount, format, dstPattern,  name, id
            });
            updateImgsShippers = true;
            return GetShipperFromName(name);
        }

        int BatchShader(std::string path, ShaderStage stage)
        {
            if (shadersNames.contains(path))
            {
                return shadersNames.at(path);
            }
            int32_t id = shaders.size();
            shaderUpdateInfos.push_back({path, stage, id});
            shadersNames.try_emplace(path, id);
            return id;
        }


        int BatchBuffer(std::string name, vk::BufferUsageFlags bufferUsageFlags,
                          vk::MemoryPropertyFlags memPropertyFlags, vk::DeviceSize deviceSize
                          ,void* data = nullptr)
        {
            if (bufferNames.contains(name))
            {
                return bufferNames.at(name);
            }
            int32_t id = buffers.size();
            bufferBatchInfos.push_back({name, deviceSize, data, bufferUsageFlags, memPropertyFlags});
            bufferNames.try_emplace(name, id);
            return id;
        }
        

        ImageView* GetImage(std::string name, vk::ImageCreateInfo imageInfo, int baseMipLevel, int baseArrayLayer)
        {
            assert(core!= nullptr &&"core must be set");
            ImageView* imageViewRef = nullptr;
            if (imagesNames.contains(name) && (imageInfo.usage & vk::ImageUsageFlagBits::eStorage))
            {
                imageViewRef = GetStorageFromName(name);
                return imageViewRef;
            }
            else if (imagesNames.contains(name))
            {
                imageViewRef = GetImageViewFromName(name);
                return imageViewRef;
            }

            auto image = std::make_unique<Image>(core->physicalDevice, core->logicalDevice.get(), imageInfo);
            if (imageInfo.usage & vk::ImageUsageFlagBits::eStorage)
            {
                int32_t id = (int32_t)storageImgsViews.size();
                assert(!storageImgsNames.contains(name) && "Img name already exist");
                storageImgsNames.try_emplace(name, (int32_t)id);

                storageImgsViews.emplace_back(std::make_unique<ImageView>(
                    core->logicalDevice.get(), image->imageData.get(),
                    baseMipLevel, imageInfo.mipLevels, baseArrayLayer,
                    imageInfo.arrayLayers, name, id));
                images.emplace_back(std::move(image));
                return storageImgsViews.back().get();
            }
            else
            {
                assert(!imagesNames.contains(name) && "Img name already exist");
                int32_t id = (int32_t)imageViews.size();
                imagesNames.try_emplace(name, (int32_t)imageViews.size());
                imageViews.emplace_back(std::make_unique<ImageView>(core->logicalDevice.get(), image->imageData.get(),
                                                                    baseMipLevel, imageInfo.mipLevels, baseArrayLayer,
                                                                    imageInfo.arrayLayers, name, id));
                images.emplace_back(std::move(image));
                return imageViews.back().get();
            }
        }
        

        Buffer* GetBuffer(std::string name, vk::BufferUsageFlags bufferUsageFlags,
                          vk::MemoryPropertyFlags memPropertyFlags, vk::DeviceSize deviceSize
                          , void* data = nullptr)
        {
            assert(core!= nullptr &&"core must be set");
            if (bufferNames.contains(name))
            {
                return GetBuffFromName(name);
            }

            auto buffer = std::make_unique<Buffer>(core->physicalDevice, core->logicalDevice.get(), bufferUsageFlags, memPropertyFlags, deviceSize,data);
            
            bufferNames.try_emplace(name, (int32_t)buffers.size());
            buffers.emplace_back(std::move(buffer));
            buffersState.push_back({VALID, deviceSize, data});
            return buffers.back().get();
        }

        StagedBuffer* GetStageBuffer(std::string name, vk::BufferUsageFlags bufferUsageFlags, vk::DeviceSize deviceSize,
                                     void* data = nullptr)
        {
            assert(core!= nullptr &&"core must be set");
            if (stagedBufferNames.contains(name))
            {
                return GetStagedBuffFromName(name);
            }

            auto buffer = std::make_unique<StagedBuffer>(
                core->physicalDevice, core->logicalDevice.get(), bufferUsageFlags, deviceSize);

            if (data != nullptr)
            {
                void* mappedMem = buffer->Map();
                memcpy(mappedMem, data, deviceSize);
                auto commandExecutor = std::make_unique<ExecuteOnceCommand>(core);
                auto commandBuffer = commandExecutor->BeginCommandBuffer();
                buffer->Unmap(commandBuffer);
                commandExecutor->EndCommandBuffer();
            }

            stagedBufferNames.try_emplace(name, (int32_t)stagedBuffers.size());
            stagedBuffers.emplace_back(std::move(buffer));
            stagedBuffersState.push_back({VALID, deviceSize});
            return stagedBuffers.back().get();
        }

        Buffer* SetBuffer(std::string name, vk::DeviceSize deviceSize, void* data)
        {
            assert(core!= nullptr &&"core must be set");
            assert(bufferNames.contains(name) && "Buffer dont exist");
            assert(!stagedBufferNames.contains(name) && "buffer is in staged buffers poll");

            
            Buffer* bufferRef = buffers.at(bufferNames.at(name)).get();
            if (deviceSize > bufferRef->deviceSize)
            {
                buffersState.at(bufferNames.at(name)) = {INVALID, deviceSize, data};
                invalidateBuffers = true;
            }
            else
            {
                //pending to handle this if is a staged resource
                Buffer* bufferRefToChange = GetBuffFromName(name);
                if (bufferRefToChange->mappedMem == nullptr)
                {
                    bufferRefToChange->Map();
                }
                memcpy(bufferRefToChange->mappedMem, data, deviceSize);
                if (bufferRefToChange->usageFlags == vk::BufferUsageFlagBits::eStorageBuffer)
                {
                    bufferRefToChange->Unmap();
                }
            }

            return buffers.at(bufferNames.at(name)).get();
        }

        StagedBuffer* SetStageBuffer(std::string name, vk::DeviceSize deviceSize, void* data)
        {
            //todo
            assert(core!= nullptr &&"core must be set");
            assert(stagedBufferNames.contains(name) && "staged buffer dont exist");
            assert(!bufferNames.contains(name) && "buffer is normal buffers poll");

            if (deviceSize > stagedBuffers.at(stagedBufferNames.at(name))->size)
            {
                stagedBuffersState.at(stagedBufferNames.at(name)) = {INVALID, deviceSize, data};
                invalidateBuffers = true;
            }
            else
            {
                StagedBuffer* buffer = GetStagedBuffFromName(name);
                void* mappedMem = buffer->Map();
                memcpy(mappedMem, data, deviceSize);
                auto commandExecutor = std::make_unique<ExecuteOnceCommand>(core);
                auto commandBuffer = commandExecutor->BeginCommandBuffer();
                buffer->Unmap(commandBuffer);
                commandExecutor->EndCommandBuffer();
            }
            return stagedBuffers.at(stagedBufferNames.at(name)).get();
        }


        void DestroyResources()
        {
            buffers.clear();
            stagedBuffers.clear();
            storageImgsViews.clear();
            imageViews.clear();
            imageShippers.clear();
            images.clear();
            samplerPool.reset();
            dsets.clear();
            dsetsIds.clear();
            freeIdsBucket.clear();
            shaders.clear();
            descriptorAllocator.reset();
            filesManager.reset();
        }

        void CreateBatchedResources()
        {
            for (auto& buffer : bufferBatchInfos)
            {
                GetBuffer(buffer.name, buffer.bufferUsageFlags, buffer.memPropertyFlags, buffer.size, buffer.data);
            }
            bufferBatchInfos.clear();
            for (auto& shaderUpdate : shaderUpdateInfos)
            {
                GetShader(shaderUpdate.path, shaderUpdate.stage);
            }
            shaderUpdateInfos.clear();
            
        }

        void UpdateImages()
        {
            if (!updateImgsShippers) { return; }
            for (int i = 0; i < imageShippers.size(); i++)
            {
                ImgUpdateInfo& updateInfo = imagesUpdateInfos[i];
                if (updateInfo.bufferState == INVALID)
                {
                    imageShippers[i]->SetDataFromPath(updateInfo.path);
                    imageShippers[i]->BuildImage(core, shipperSampler, updateInfo.arrayLayersCount,
                                                 updateInfo.mipsCount,
                                                 updateInfo.format, updateInfo.dstPattern, updateInfo.name,
                                                 updateInfo.id);
                    updateInfo.bufferState = VALID;
                }
            }
        }

        void UpdateBuffers()
        {
            if (!invalidateBuffers) { return; }
            for (auto& name : bufferNames)
            {
                BufferUpdateInfo& bufferUpdateInfo = buffersState.at(name.second);
                if (bufferUpdateInfo.state == INVALID)
                {
                    buffers.at(bufferNames.at(name.first)).reset(new Buffer(
                        core->physicalDevice, core->logicalDevice.get(),
                        buffers.at(name.second)->usageFlags, buffers.at(name.second)->memPropertyFlags,
                        bufferUpdateInfo.size,
                        bufferUpdateInfo.data));
                    bufferUpdateInfo.state = VALID;
                }
            }
            auto commandExecutor = std::make_unique<ExecuteOnceCommand>(core);
            auto commandBuffer = commandExecutor->BeginCommandBuffer();
            for (auto& name : stagedBufferNames)
            {
                BufferUpdateInfo& bufferUpdateInfo = stagedBuffersState.at(name.second);
                if (bufferUpdateInfo.state == INVALID)
                {
                    stagedBuffers.at(stagedBufferNames.at(name.first)).reset(new StagedBuffer(
                        core->physicalDevice, core->logicalDevice.get(),
                        stagedBuffers.at(name.second)->deviceBuffer->usageFlags,
                        bufferUpdateInfo.size));
                    StagedBuffer* buffer = GetStagedBuffFromName(name.first);
                    void* mappedMem = buffer->Map();
                    memcpy(mappedMem, bufferUpdateInfo.data, bufferUpdateInfo.size);
                    buffer->Unmap(commandBuffer);

                    bufferUpdateInfo.state = VALID;
                }
            }
            commandExecutor->EndCommandBuffer();
            Notify();
            invalidateBuffers = false;
        }

        void EndFrameDynamicUpdates(vk::CommandBuffer commandBuffer)
        {
            BufferAccessPattern src = GetSrcBufferAccessPattern(BufferUsageTypes::B_GRAPHICS_WRITE);
            BufferAccessPattern dst = GetSrcBufferAccessPattern(BufferUsageTypes::B_TRANSFER_DST);
            CreateMemBarrier(src, dst, commandBuffer);
            for (int i = 0; i < storageImgsToClear.size(); ++i)
            {
                ImageView* imageView = GetStorageFromName(storageImgsToClear[i]);
                vk::ClearColorValue clearColor(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});

                commandBuffer.clearColorImage(
                    imageView->imageData->imageHandle,
                    vk::ImageLayout::eGeneral,
                    clearColor,
                    imageView->GetSubresourceRange()
                );
            }
            storageImgsToClear.clear();
        }


        void RequestStorageImageClear(std::string name)
        {
            if (!storageImgsNames.contains(name))
            {
                SYSTEMS::Logger::GetInstance()->LogMessage("Storage Img with name does not exist: " + name);
                return;
            }
            storageImgsToClear.push_back(name);
        }

        Shader* GetShader(std::string path, ShaderStage stage)
        {
            if (shadersNames.contains(path))
            {
                return shaders.at(shadersNames.at(path)).get();
            }
            int id = shaders.size();
            
            shaders.emplace_back(std::make_unique<Shader>(core->logicalDevice.get(), path, stage));
            
            shadersNames.try_emplace(shaders.back()->spirvPath, id);
            
            Shader* shader = shaders.back().get();
            return shader;
        }
        Shader* GetShaderFromId(int id)
        {
            for (auto shader : shadersNames)
            {
                if (shader.second == id)
                {
                    return shaders.at(id).get();
                }
            }
            assert(false && "invalid shader id");
            return nullptr;
        }
        Shader* CreateDefaultShader(std::string name, ShaderStage stage, ShaderCompiler compiler)
        {
            std::filesystem::path targetPath;
            std::filesystem::path templatePath;
            switch (compiler)
            {
            case C_GLSL:
                switch (stage)
                {
                case S_VERT:
                    if (std::filesystem::path(name).extension() != ".vert")
                    {
                        name += ".vert";
                    }
                    templatePath = SYSTEMS::OS::GetInstance()->glslShadersTemplatePath /"tVert.vert";
                    break;
                case S_FRAG:
                    if (std::filesystem::path(name).extension() != ".frag")
                    {
                        name += ".frag";
                    }
                    templatePath = SYSTEMS::OS::GetInstance()->glslShadersTemplatePath / "tFrag.frag";
                    break;

                case S_COMP:
                    if (std::filesystem::path(name).extension() != ".comp")
                    {
                        name += ".comp";
                    }
                    templatePath = SYSTEMS::OS::GetInstance()->glslShadersTemplatePath / "tComp.comp";
                    break;
                case S_TESS_CONTROL:
                    if (std::filesystem::path(name).extension() != ".tesc")
                    {
                        name += ".tesc";
                    }
                    templatePath = SYSTEMS::OS::GetInstance()->glslShadersTemplatePath / "tTesc.tesc";
                    break;
                case S_TESS_EVAL:
                    if (std::filesystem::path(name).extension() != ".tese")
                    {
                        name += ".tese";
                    }
                    templatePath = SYSTEMS::OS::GetInstance()->glslShadersTemplatePath / "tTese.tese";
                    break;
                case S_GEOM:
                    if (std::filesystem::path(name).extension() != ".geom")
                    {
                        name += ".geom";
                    }
                    templatePath = SYSTEMS::OS::GetInstance()->glslShadersTemplatePath / "tGeom.geom";
                    break;
                case S_UNKNOWN:
                    break;
                }
                targetPath = SYSTEMS::OS::GetInstance()->shadersPath / "glsl" / "generated" / name;
                break;
            case C_SLANG:
                if (std::filesystem::path(name).extension() != ".slang")
                {
                    name += ".slang";
                }
                targetPath = SYSTEMS::OS::GetInstance()->shadersPath / "slang" / "generated" / name;
                templatePath = SYSTEMS::OS::GetInstance()->slangShadersTemplatePath;
                if (stage == ShaderStage::S_COMP)
                {
                    templatePath /= "compute.slang";
                }
                else
                {
                    templatePath /= "quad.slang";
                }
                break;
            default:
                assert(false);
                break;
            }
           
            if (std::filesystem::exists(targetPath))
            {
                return GetShader(targetPath.string(), stage);
            }
            
            SYSTEMS::OS::GetInstance()->CopyFileInto(templatePath.string(), targetPath.string());
            
            std::string spirvPath = ConvertShaderPathToSpirv(targetPath.string(), stage);
            Shader* shader= GetShader(targetPath.string(), stage);

            //danger
            // filesManager->AddPathToDelete(targetPath.string());
            // filesManager->AddPathToDelete(spirvPath);
            
            return shader;
        }

        VertexInput* GetVertexInput(std::string name)
        {
            if (verticesInputsNames.contains(name))
            {
                return verticesInputs.at(verticesInputsNames.at(name)).get();
            }
            verticesInputsNames.try_emplace(name, verticesInputs.size());
            verticesInputs.emplace_back(std::make_unique<VertexInput>());
            VertexInput* vertexInput = verticesInputs.back().get();
            return vertexInput;
        }
        VertexInput* GetVertexInputFromId(int id)
        {
            for (auto& name : verticesInputsNames)
            {
                if (name.second == id)
                {
                    return verticesInputs.at(name.second).get();
                }
            }
            assert(false && "invalid vertex id");
        }
        
        

        DsetsInfo AllocateDset(vk::DescriptorSetLayout dstSetLayout)
        {
            if (!freeIdsBucket.empty())
            {
                for (auto& id : freeIdsBucket)
                {
                    SYSTEMS::Logger::GetInstance()->Log("Free Id: " + std::to_string(id));
                }
                int32_t id = freeIdsBucket.front();
                freeIdsBucket.erase(freeIdsBucket.begin());
                dsets.at(id) = descriptorAllocator->Allocate(core->logicalDevice.get(), dstSetLayout);
                return DsetsInfo{dsets.at(id).get(), id};
            }
            else
            {
                int32_t id = dsets.size();
                dsetsIds.insert(id);
                dsets.emplace_back(descriptorAllocator->Allocate(core->logicalDevice.get(), dstSetLayout));
                return DsetsInfo{dsets.back().get(), id};
            }
        }

        void DeallocateDset(int id)
        {
            if (dsetsIds.contains(id))
            {
                dsets.at(id).reset();
                freeIdsBucket.emplace_back(id);
            }
        }


        ImageView* GetImageViewFromName(std::string name)
        {
            if (!imagesNames.contains(name))
            {
                SYSTEMS::Logger::GetInstance()->Log("Img View With Name: " + name + "Does not exist");
                return nullptr;
            }
            return imageViews.at(imagesNames.at(name)).get();
        }

        ImageShipper* GetShipperFromName(std::string name)
        {
            if (!imagesShippersNames.contains(name))
            {
                SYSTEMS::Logger::GetInstance()->Log("Img Shipper With Name: " + name + "Does not exist");
                return nullptr;
            }
            return imageShippers.at(imagesShippersNames.at(name)).get();
        }

        ImageView* GetStorageFromName(std::string name)
        {
            if (!storageImgsNames.contains(name))
            {
                return nullptr;
            }
            return storageImgsViews.at(storageImgsNames.at(name)).get();
        }

        ImageView* GetImageViewFromId(int id)
        {
            for (auto& imageName : imagesNames)
            {
                if (imageName.second == id)
                {
                    return imageViews.at(id).get();
                }
            }
            SYSTEMS::Logger::GetInstance()->Log("Img View With id: " + std::to_string(id) + "Does not exist");
            return nullptr;
        }

        ImageShipper* GetShipperFromId(int id)
        {
            for (auto& imageName : imagesShippersNames)
            {
                if (imageName.second == id)
                {
                    return imageShippers.at(id).get();
                }
            }
            SYSTEMS::Logger::GetInstance()->Log("Img View With id: " + std::to_string(id) + "Does not exist");
            return nullptr;
        }

        ImageView* GetStorageFromId(int id)
        {
            for (auto& imageName : storageImgsNames)
            {
                if (imageName.second == id)
                {
                    return storageImgsViews.at(id).get();
                }
            }
            SYSTEMS::Logger::GetInstance()->Log("Img View With id: " + std::to_string(id) + "Does not exist");
            return nullptr;
        }
        
        Buffer* GetBuffFromName(std::string name)
        {
            if (!bufferNames.contains(name))
            {
                return nullptr;
            }
            return buffers.at(bufferNames.at(name)).get();
        }

        StagedBuffer* GetStagedBuffFromName(std::string name)
        {
            if (!stagedBufferNames.contains(name))
            {
                return nullptr;
            }
            return stagedBuffers.at(stagedBufferNames.at(name)).get();
        }

        int GetShipperID(std::string name)
        {
            return imagesShippersNames.at(name);
        }

        static ResourcesManager* GetInstance(Core* coreRef = nullptr)
        {
            if (instance == nullptr && coreRef != nullptr)
            {
                instance = new ResourcesManager(coreRef);
            }
            return instance;
        }

        ResourcesManager(Core* coreRefs)
        {
            this->core = coreRefs;
            stagedBuffers.reserve(BASE_SIZE);
            buffers.reserve(BASE_SIZE);
            storageImgsViews.reserve(BASE_SIZE);
            imageViews.reserve(BASE_SIZE);
            images.reserve(BASE_SIZE);
            filesManager = std::make_unique<SYSTEMS::FilesManager>();
            descriptorAllocator = std::make_unique<DescriptorAllocator>();
            descriptorAllocator->BeginPool(core->logicalDevice.get(), 100, poolSizeRatios);

            samplerPool = std::make_unique<SamplerPool>();

            shipperSampler = samplerPool->AddSampler(core->logicalDevice.get(), vk::SamplerAddressMode::eRepeat,
                                                     vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
            

            std::string defaultTexturePath = SYSTEMS::OS::GetInstance()->GetEngineResourcesPath() +
                "\\Images\\default_texture.jpg";

            ImageShipper* shipper = GetShipper("default_tex", defaultTexturePath, 1, 1, g_ShipperFormat,
                                               LayoutPatterns::GRAPHICS_READ);

            auto imageInfo = ENGINE::Image::CreateInfo2d(
                glm::uvec2(core->swapchainRef->extent.width, core->swapchainRef->extent.height), 1, 1,
                ENGINE::g_32bFormat,
                vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);

            ImageView* defaultStorage = GetImage("default_storage", imageInfo, 0, 0);
        }

        ~ResourcesManager() = default;

        void Attach(SYSTEMS::Watcher* watcher) override
        {
            watchers.push_back(watcher);
        }

        void Detach(SYSTEMS::Watcher* watcher) override
        {
            watchers.erase(std::remove(watchers.begin(), watchers.end(), watcher), watchers.end());
        }

        void Notify() override
        {
            for (auto& watcher : watchers)
            {
                watcher->UpdateWatcher();
            }
        }

        std::vector<SYSTEMS::Watcher*> watchers;

        std::unordered_map<std::string, int32_t> bufferNames;
        std::unordered_map<std::string, int32_t> stagedBufferNames;
        std::unordered_map<std::string, int32_t> imagesNames;
        std::unordered_map<std::string, int32_t> storageImgsNames;
        std::unordered_map<std::string, int32_t> imagesShippersNames;
        std::unordered_map<std::string, int32_t> shadersNames;
        std::unordered_map<std::string, int32_t> verticesInputsNames;

        std::set<int32_t> dsetsIds;
        std::deque<int32_t> freeIdsBucket;

        std::vector<std::unique_ptr<Buffer>> buffers;
        std::vector<std::unique_ptr<StagedBuffer>> stagedBuffers;
        std::vector<std::unique_ptr<ImageView>> imageViews;
        std::vector<std::unique_ptr<ImageView>> storageImgsViews;
        std::vector<std::unique_ptr<ImageShipper>> imageShippers;
        std::vector<std::unique_ptr<Image>> images;
        std::vector<std::string> storageImgsToClear;
        std::unique_ptr<SamplerPool> samplerPool;
        std::unique_ptr<DescriptorAllocator> descriptorAllocator;
        std::vector<vk::UniqueDescriptorSet> dsets;
        std::vector<std::unique_ptr<Shader>> shaders;
        std::vector<std::unique_ptr<VertexInput>> verticesInputs;


        std::vector<ImgUpdateInfo> imagesUpdateInfos;
        std::vector<BufferUpdateInfo> buffersState;
        std::vector<BufferUpdateInfo> stagedBuffersState;
        
        std::vector<ShaderBatchInfo> shaderUpdateInfos;
        std::vector<BufferBatchInfo> bufferBatchInfos;

        std::vector<ENGINE::DescriptorAllocator::PoolSizeRatio> poolSizeRatios = {
            {vk::DescriptorType::eSampler, 1.5f},
            {vk::DescriptorType::eStorageBuffer, 1.5f},
            {vk::DescriptorType::eUniformBuffer, 1.5f},
            {vk::DescriptorType::eStorageImage, 1.5f},
        };
        std::unique_ptr<SYSTEMS::FilesManager> filesManager;

        Sampler* shipperSampler;

        bool invalidateBuffers = false;
        bool updateImgsShippers = false;
        bool updateImgs = false;

        Core* core;
        static ResourcesManager* instance;
    };

    ResourcesManager* ResourcesManager::instance = nullptr;
}


#endif //RESOURCESMANAGER_HPP
