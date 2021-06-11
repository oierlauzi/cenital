#include <Overlays/Keyer.h>

#include <Shapes.h>

#include <zuazo/Signal/Input.h>
#include <zuazo/Signal/Output.h>
#include <zuazo/Utils/StaticId.h>
#include <zuazo/Utils/Hasher.h>
#include <zuazo/Utils/Pool.h>
#include <zuazo/Graphics/StagedBuffer.h>
#include <zuazo/Graphics/UniformBuffer.h>
#include <zuazo/Graphics/CommandBufferPool.h>
#include <zuazo/Graphics/ColorTransfer.h>
#include <zuazo/Math/Geometry.h>
#include <zuazo/Math/Absolute.h>
#include <zuazo/Math/LoopBlinn/OutlineProcessor.h>

#include <utility>
#include <memory>
#include <unordered_map>

namespace Cenital::Overlays {

using namespace Zuazo;

struct KeyerImpl {
	struct Open {
		struct Vertex {
			Vertex(	const Math::Vec2f& position, 
					const Math::Vec2f& texCoord, 
					const Math::Vec3f& klm ) noexcept
				: position(position)
				, texCoord(texCoord)
				, klm(klm)
			{
			}

			Math::Vec2f position;
			Math::Vec2f texCoord;
			Math::Vec3f klm;
		};

		struct FragmentConstants {
			FragmentConstants() = default;
			FragmentConstants(	uint32_t sampleMode,
								VkBool32 sameKeyFill, 
								VkBool32 lumaKeyEnabled, 
								VkBool32 chromaKeyEnabled,
								int32_t linearKeyType ) noexcept
				: sampleMode(sampleMode)
				, sameKeyFill(sameKeyFill)
				, lumaKeyEnabled(lumaKeyEnabled)
				, chromaKeyEnabled(chromaKeyEnabled)
				, linearKeyType(linearKeyType)
			{
			}

			uint32_t		sampleMode;
			VkBool32		sameKeyFill;
			VkBool32		lumaKeyEnabled;
			VkBool32		chromaKeyEnabled;
			int32_t			linearKeyType;

		};

		using Index = uint16_t;

		enum VertexBufferBindings {
			VERTEX_BUFFER_BINDING,

			VERTEX_BUFFER_COUNT
		};

		enum VertexLayout {
			VERTEX_LOCATION_POSITION,
			VERTEX_LOCATION_TEXCOORD,
			VERTEX_LOCATION_KLM,

			VERTEX_LOCATION_COUNT
		};

		enum DescriptorSets {
			DESCRIPTOR_SET_RENDERER = RendererBase::DESCRIPTOR_SET,
			DESCRIPTOR_SET_KEYER,
			DESCRIPTOR_SET_KEYFRAME,
			DESCRIPTOR_SET_FILLFRAME,

			DESCRIPTOR_SET_COUNT
		};

		enum DescriptorBindings {
			DESCRIPTOR_BINDING_MODEL_MATRIX,
			DESCRIPTOR_BINDING_LAYERDATA,

			DESCRIPTOR_COUNT
		};

		enum FragmentConstantId {
			FRAGMENT_CONSTANT_ID_SAMPLE_MODE,
			FRAGMENT_CONSTANT_ID_SAME_KEY_FILL,
			FRAGMENT_CONSTANT_ID_LUMA_KEY_ENABLED,
			FRAGMENT_CONSTANT_ID_CHROMA_KEY_ENABLED,
			FRAGMENT_CONSTANT_ID_LINEAR_KEY_TYPE,

			FRAGMENT_CONSTANT_ID_COUNT
		};

		enum LayerDataUniforms {
			LAYERDATA_UNIFORM_LUMAKEY_MIN_THRESHOLD,
			LAYERDATA_UNIFORM_LUMAKEY_MAX_THRESHOLD,
			LAYERDATA_UNIFORM_CHROMAKEY_HUE,
			LAYERDATA_UNIFORM_CHROMAKEY_HUE_THRESHOLD,
			LAYERDATA_UNIFORM_CHROMAKEY_HUE_SMOOTHNESS,
			LAYERDATA_UNIFORM_CHROMAKEY_SATURATION_THRESHOLD,
			LAYERDATA_UNIFORM_CHROMAKEY_SATURATION_SMOOTHNESS,
			LAYERDATA_UNIFORM_CHROMAKEY_VALUE_THRESHOLD,
			LAYERDATA_UNIFORM_CHROMAKEY_VALUE_SMOOTHNESS,
			LAYERDATA_UNIFORM_OPACITY,

			LAYERDATA_UNIFORM_COUNT
		};

		static constexpr std::array<Utils::Area, LAYERDATA_UNIFORM_COUNT> LAYERDATA_UNIFORM_LAYOUT = {
			Utils::Area(0*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_LUMAKEY_MIN_THRESHOLD 
			Utils::Area(1*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_LUMAKEY_MAX_THRESHOLD 
			Utils::Area(4*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_HUE 
			Utils::Area(5*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_HUE_THRESHOLD 
			Utils::Area(6*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_HUE_SMOOTHNESS 
			Utils::Area(7*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_SATURATION_THRESHOLD 
			Utils::Area(8*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_SATURATION_SMOOTHNESS 
			Utils::Area(9*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_VALUE_THRESHOLD 
			Utils::Area(10*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_CHROMAKEY_VALUE_SMOOTHNESS 
			Utils::Area(12*sizeof(float),	sizeof(float)),	//LAYERDATA_UNIFORM_OPACITY 
		};

		static constexpr std::array<vk::SpecializationMapEntry, FRAGMENT_CONSTANT_ID_COUNT> FRAGMENT_SPECIALIZATION_LAYOUT = {
			vk::SpecializationMapEntry(
				FRAGMENT_CONSTANT_ID_SAMPLE_MODE,
				offsetof(FragmentConstants, sampleMode),
				sizeof(FragmentConstants::sampleMode)
			),
			vk::SpecializationMapEntry(
				FRAGMENT_CONSTANT_ID_SAME_KEY_FILL,
				offsetof(FragmentConstants, sameKeyFill),
				sizeof(FragmentConstants::sameKeyFill)
			),
			vk::SpecializationMapEntry(
				FRAGMENT_CONSTANT_ID_LUMA_KEY_ENABLED,
				offsetof(FragmentConstants, lumaKeyEnabled),
				sizeof(FragmentConstants::lumaKeyEnabled)
			),
			vk::SpecializationMapEntry(
				FRAGMENT_CONSTANT_ID_CHROMA_KEY_ENABLED,
				offsetof(FragmentConstants, chromaKeyEnabled),
				sizeof(FragmentConstants::chromaKeyEnabled)
			),
			vk::SpecializationMapEntry(
				FRAGMENT_CONSTANT_ID_LINEAR_KEY_TYPE,
				offsetof(FragmentConstants, linearKeyType),
				sizeof(FragmentConstants::linearKeyType)
			),
		};

		struct Resources {
			Resources(	Graphics::UniformBuffer uniformBuffer,
						vk::UniqueDescriptorPool descriptorPool )
				: vertexBuffer()
				, indexBuffer()
				, uniformBuffer(std::move(uniformBuffer))
				, descriptorPool(std::move(descriptorPool))
			{
			}

			~Resources() = default;

			Graphics::StagedBuffer								vertexBuffer;
			Graphics::StagedBuffer								indexBuffer;
			Graphics::UniformBuffer								uniformBuffer;
			vk::UniqueDescriptorPool							descriptorPool;
		};

		const Graphics::Vulkan&								vulkan;

		std::shared_ptr<Resources>							resources;
		vk::DescriptorSet									descriptorSet;

		Math::LoopBlinn::OutlineProcessor<float, uint16_t>	outlineProcessor;
		Graphics::Frame::Geometry							frameGeometry;

		bool												flushVertexBuffer;
		bool												flushIndexBuffer;

		FragmentConstants									fragmentConstants;											
		vk::DescriptorSetLayout								keyFrameDescriptorSetLayout;
		vk::DescriptorSetLayout								fillFrameDescriptorSetLayout;
		vk::PipelineLayout									pipelineLayout;
		vk::Pipeline										pipeline;

		Open(	const Graphics::Vulkan& vulkan,
				Math::Vec2f size,
				ScalingMode scalingMode ) 
			: vulkan(vulkan)
			, resources(Utils::makeShared<Resources>(	createUniformBuffer(vulkan),
														createDescriptorPool(vulkan) ))
			, descriptorSet(createDescriptorSet(vulkan, *resources->descriptorPool))
			, outlineProcessor()
			, frameGeometry(scalingMode, size)
			, flushVertexBuffer(false)
			, flushIndexBuffer(false)
			, fragmentConstants()
			, keyFrameDescriptorSetLayout()
			, fillFrameDescriptorSetLayout()
			, pipelineLayout()
			, pipeline()
		{
			resources->uniformBuffer.writeDescirptorSet(vulkan, descriptorSet);
		}

		~Open() {
			resources->vertexBuffer.waitCompletion(vulkan);
			resources->uniformBuffer.waitCompletion(vulkan);
		}

		void recreate() {
			//Force pipeline creation
			keyFrameDescriptorSetLayout = nullptr;
		}

		void draw(	Graphics::CommandBuffer& cmd, 
					const Video& keyFrame,
					const Video& fillFrame,
					ScalingFilter filter,
					vk::RenderPass renderPass,
					BlendingMode blendingMode,
					RenderingLayer renderingLayer ) 
		{				
			assert(resources);			
			assert(keyFrame);
			assert(fillFrame);

			//Update the vertex buffer if needed
			if(frameGeometry.useFrame(*fillFrame)) {
				//Size has changed. Recalculate the vertex buffer
				flushVertexBuffer = true;
			}

			//Upload vertex and index data if necessary
			fillVertexBuffer();
			fillIndexBuffer();

			//Only draw if geometry is defined
			if(resources->indexBuffer.size()) {
				assert(resources->vertexBuffer.size());

				//Flush the unform buffer
				resources->uniformBuffer.flush(vulkan);

				//Configure the samplers for propper operation
				configureSamplers(*keyFrame, *fillFrame, filter, renderPass, blendingMode, renderingLayer);
				assert(keyFrameDescriptorSetLayout);
				assert(fillFrameDescriptorSetLayout);
				assert(pipelineLayout);
				assert(pipeline);

				//Bind the pipeline and its descriptor sets
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

				cmd.bindVertexBuffers(
					VERTEX_BUFFER_BINDING,											//Binding
					resources->vertexBuffer.getBuffer(),							//Vertex buffers
					0UL																//Offsets
				);

				cmd.bindIndexBuffer(
					resources->indexBuffer.getBuffer(),								//Index buffer
					0,																//Offset
					vk::IndexType::eUint16											//Index type
				);

				cmd.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,								//Pipeline bind point
					pipelineLayout,													//Pipeline layout
					DESCRIPTOR_SET_KEYER,											//First index
					descriptorSet,													//Descriptor sets
					{}																//Dynamic offsets
				);

				keyFrame->bind(
					cmd.get(), 														//Commandbuffer
					pipelineLayout, 												//Pipeline layout
					DESCRIPTOR_SET_KEYFRAME, 										//Descriptor set index
					filter															//Filter
				);
				fillFrame->bind(
					cmd.get(), 														//Commandbuffer
					pipelineLayout, 												//Pipeline layout
					DESCRIPTOR_SET_FILLFRAME, 										//Descriptor set index
					filter															//Filter
				);

				//Draw the frame and finish recording
				cmd.drawIndexed(
					resources->indexBuffer.size() / sizeof(Index),					//Index count
					1, 																//Instance count
					0, 																//First index
					0, 																//First vertex
					0																//First instance
				);

				//Add the dependencies to the command buffer
				cmd.addDependencies({ resources, keyFrame, fillFrame });

			}		
		}

		void setCrop(Utils::BufferView<const Shape> crop) {
			outlineProcessor.clear();
			outlineProcessor.addOutline(crop);
			
			flushIndexBuffer = true;
			flushVertexBuffer = true;
		}



		void updateLumaKeyEnabledConstant(VkBool32 ena) {
			updateFragmentConstant(FRAGMENT_CONSTANT_ID_LUMA_KEY_ENABLED, ena);
		}

		void updateChromaKeyEnabledConstant(VkBool32 ena) {
			updateFragmentConstant(FRAGMENT_CONSTANT_ID_CHROMA_KEY_ENABLED, ena);
		}

		void updateLinearKeyTypeConstant(bool enabled, bool inverted, Keyer::LinearKeyChannel channel) {
			updateFragmentConstant(FRAGMENT_CONSTANT_ID_LINEAR_KEY_TYPE, calculateLinearKeyType(enabled, inverted, channel));
		}


		void updateModelMatrixUniform(const Math::Transformf& transform) {
			assert(resources);
			resources->uniformBuffer.waitCompletion(vulkan);

			const auto mtx = transform.calculateMatrix();
			resources->uniformBuffer.write(
				vulkan,
				DESCRIPTOR_BINDING_MODEL_MATRIX,
				&mtx,
				sizeof(mtx)
			);
		}


		void updateLumaKeyThresholdUniform(bool inverted, float minThreshold, float maxThreshold) {
			if(inverted) {
				minThreshold = -minThreshold;
				maxThreshold = -maxThreshold;
			}

			updateFragmentUniform(LAYERDATA_UNIFORM_LUMAKEY_MIN_THRESHOLD, minThreshold);
			updateFragmentUniform(LAYERDATA_UNIFORM_LUMAKEY_MAX_THRESHOLD, maxThreshold);
		}

		void updateChromaKeyHueUniform(float hue) {
			hue = Math::mod(Math::mod(hue, 360.0f) + 360.0f, 360.0f); //First positive turn
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_HUE, hue);
		}

		void updateChromaKeyHueThresholdUniform(float threshold) {
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_HUE_THRESHOLD, threshold);
		}

		void updateChromaKeyHueSmoothnessUniform(float smoothness) {
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_HUE_SMOOTHNESS, smoothness);
		}

		void updateChromaKeySaturationThresholdUniform(float threshold) {
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_SATURATION_THRESHOLD, threshold);
		}

		void updateChromaKeySaturationSmoothnessUniform(float smoothness) {
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_SATURATION_SMOOTHNESS, smoothness);
		}

		void updateChromaKeyValueThresholdUniform(float threshold) {
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_VALUE_THRESHOLD, threshold);
		}

		void updateChromaKeyValueSmoothnessUniform(float smoothness) {
			updateFragmentUniform(LAYERDATA_UNIFORM_CHROMAKEY_VALUE_SMOOTHNESS, smoothness);
		}


		void updateOpacityUniform(float opa) {
			updateFragmentUniform(LAYERDATA_UNIFORM_OPACITY, opa);
		}

	private:
		void configureSamplers(	const Graphics::Frame& keyFrame, 
								const Graphics::Frame& fillFrame, 
								ScalingFilter filter,
								vk::RenderPass renderPass,
								BlendingMode blendingMode,
								RenderingLayer renderingLayer ) 
		{
			const auto newKeyDescriptorSetLayout = keyFrame.getDescriptorSetLayout(filter);
			const auto newFillDescriptorSetLayout = fillFrame.getDescriptorSetLayout(filter);
			const auto newSampleMode = fillFrame.getSamplingMode(filter);
			const auto newSameKeyFill = &keyFrame == &fillFrame; //Check if the pointed is the same

			if(	keyFrameDescriptorSetLayout != newKeyDescriptorSetLayout ||
				fillFrameDescriptorSetLayout != newFillDescriptorSetLayout ||
				fragmentConstants.sampleMode != newSampleMode ||
				fragmentConstants.sameKeyFill != newSameKeyFill ) 
			{
				keyFrameDescriptorSetLayout = newKeyDescriptorSetLayout;
				fillFrameDescriptorSetLayout = newFillDescriptorSetLayout;
				fragmentConstants.sampleMode = newSampleMode;
				fragmentConstants.sameKeyFill = newSameKeyFill;

				//Recreate stuff
				pipelineLayout = createPipelineLayout(vulkan, keyFrameDescriptorSetLayout, fillFrameDescriptorSetLayout);
				pipeline = createPipeline(
					vulkan, 
					pipelineLayout, 
					renderPass,
					blendingMode, 
					renderingLayer,
					fragmentConstants
				);
			}
		}

		void fillVertexBuffer() {
			assert(resources);

			if(flushVertexBuffer) {
				const auto surfaceSize = frameGeometry.calculateSurfaceSize();
				const auto& vertices = outlineProcessor.getVertices();

				//Wait for any previous transfers
				resources->vertexBuffer.waitCompletion(vulkan);

				//Recreate if size has changed
				if(resources->vertexBuffer.size() != vertices.size()*sizeof(Vertex)) {
					resources->vertexBuffer = createVertexBuffer(vulkan, vertices.size());
				}

				//Obtain the buffer data
				Utils::BufferView<Vertex> vertexBufferData(
					reinterpret_cast<Vertex*>(resources->vertexBuffer.data()),
					resources->vertexBuffer.size() / sizeof(Vertex)
				);

				//Ensure the size is correct
				assert(vertexBufferData.size() == vertices.size());

				//Copy the data
				for(size_t i = 0; i < vertexBufferData.size(); ++i) {
					//Obtain the interpolation parameter based on the position
					const auto t = Math::ilerp(
						-surfaceSize.first / 2.0f, 
						+surfaceSize.first / 2.0f, 
						vertices[i].pos
					);

					//Interpolate the texture coordinates
					const auto texCoord = Math::lerp(
						(Math::Vec2f(1.0f) - surfaceSize.second) / 2.0f,
						(Math::Vec2f(1.0f) + surfaceSize.second) / 2.0f,
						t
					);

					vertexBufferData[i] = Vertex(
						vertices[i].pos,
						texCoord,
						vertices[i].klm
					);

				}

				//Flush the buffer
				resources->vertexBuffer.flushData(
					vulkan, 
					vulkan.getTransferQueueIndex(), 
					vk::AccessFlagBits::eVertexAttributeRead,
					vk::PipelineStageFlagBits::eVertexInput
				);

				flushVertexBuffer = false;
			}

			assert(!flushVertexBuffer);
		}

		void fillIndexBuffer() {
			assert(resources);

			if(flushIndexBuffer) {
				const auto& indices = outlineProcessor.getIndices();

				//Wait for any previous transfers
				resources->indexBuffer.waitCompletion(vulkan);

				//Recreate if size has changed
				if(resources->indexBuffer.size() != indices.size()*sizeof(Index)) {
					resources->indexBuffer = createIndexBuffer(vulkan, indices.size());
				}

				//Ensure the size is correct
				assert(resources->indexBuffer.size() == indices.size()*sizeof(Index));

				//Copy the data
				std::memcpy(
					resources->indexBuffer.data(), 
					indices.data(), 
					indices.size()*sizeof(Index)
				);

				//Flush the buffer
				resources->indexBuffer.flushData(
					vulkan, 
					vulkan.getTransferQueueIndex(), 
					vk::AccessFlagBits::eIndexRead,
					vk::PipelineStageFlagBits::eVertexInput
				);

				flushIndexBuffer = false;
			}

			assert(!flushIndexBuffer);
		}



		template<typename T>
		void updateFragmentConstant(FragmentConstantId id, const T& value) {
			//Write the constant at the correct spot
			assert(sizeof(value) == FRAGMENT_SPECIALIZATION_LAYOUT[id].size);
			const auto data = reinterpret_cast<std::byte*>(&fragmentConstants);
			*reinterpret_cast<T*>(data + FRAGMENT_SPECIALIZATION_LAYOUT[id].offset) = value;

			recreate();
		}

		template<typename T>
		void updateFragmentUniform(LayerDataUniforms binding, const T& value) {
			assert(resources);
			resources->uniformBuffer.waitCompletion(vulkan);			

			resources->uniformBuffer.write(
				vulkan,
				DESCRIPTOR_BINDING_LAYERDATA,
				&value,
				sizeof(value),
				LAYERDATA_UNIFORM_LAYOUT[binding].offset()
			);
		}


		static int32_t calculateLinearKeyType(bool enabled, bool inverted, Keyer::LinearKeyChannel channel) {
			int32_t result;

			if(enabled) {
				switch (channel) {
				case Keyer::LinearKeyChannel::keyR:		result = 0x01; break;
				case Keyer::LinearKeyChannel::keyG:		result = 0x02; break;
				case Keyer::LinearKeyChannel::keyB:		result = 0x03; break;
				case Keyer::LinearKeyChannel::keyA:		result = 0x04; break;
				case Keyer::LinearKeyChannel::keyY:		result = 0x05; break;
				
				case Keyer::LinearKeyChannel::fillR:	result = 0x06; break;
				case Keyer::LinearKeyChannel::fillG:	result = 0x07; break;
				case Keyer::LinearKeyChannel::fillB:	result = 0x08; break;
				case Keyer::LinearKeyChannel::fillA:	result = 0x09; break;
				case Keyer::LinearKeyChannel::fillY:	result = 0x0A; break;

				default:								result = 0x00; break; //Not expected	
				}

				if(inverted) {
					result = -result;
				}
			} else {
				result = 0;
			}

			return result;
		}


		static Graphics::StagedBuffer createVertexBuffer(const Graphics::Vulkan& vulkan, size_t vertexCount) {
			if(vertexCount > 0) {
				return Graphics::StagedBuffer(
					vulkan,
					vk::BufferUsageFlagBits::eVertexBuffer,
					sizeof(Vertex) * vertexCount
				);
			} else {
				return {};
			}
		}

		static Graphics::StagedBuffer createIndexBuffer(const Graphics::Vulkan& vulkan, size_t indexCount) {
			if(indexCount > 0) {
				return Graphics::StagedBuffer(
					vulkan,
					vk::BufferUsageFlagBits::eIndexBuffer,
					sizeof(Index) * indexCount
				);
			} else {
				return {};
			}
		}

		static vk::DescriptorSetLayout getDescriptorSetLayout(	const Graphics::Vulkan& vulkan) 
		{
			static const Utils::StaticId id;
			auto result = vulkan.createDescriptorSetLayout(id);

			if(!result) {
				//Create the bindings
				const std::array bindings = {
					vk::DescriptorSetLayoutBinding(	//UBO binding
						DESCRIPTOR_BINDING_MODEL_MATRIX,				//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eVertex,				//Shader stage
						nullptr											//Immutable samplers
					), 
					vk::DescriptorSetLayoutBinding(	//UBO binding
						DESCRIPTOR_BINDING_LAYERDATA,					//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eFragment,				//Shader stage
						nullptr											//Immutable samplers
					), 
				};

				const vk::DescriptorSetLayoutCreateInfo createInfo(
					{},
					bindings.size(), bindings.data()
				);

				result = vulkan.createDescriptorSetLayout(id, createInfo);
			}

			return result;
		}

		static Utils::BufferView<const std::pair<uint32_t, size_t>> getUniformBufferSizes() noexcept {
			static const std::array uniformBufferSizes = {
				std::make_pair<uint32_t, size_t>(DESCRIPTOR_BINDING_MODEL_MATRIX, 	sizeof(Math::Mat4x4f) ),
				std::make_pair<uint32_t, size_t>(DESCRIPTOR_BINDING_LAYERDATA,		LAYERDATA_UNIFORM_LAYOUT.back().end() )
			};

			return uniformBufferSizes;
		}

		static Graphics::UniformBuffer createUniformBuffer(const Graphics::Vulkan& vulkan) {
			return Graphics::UniformBuffer(vulkan, getUniformBufferSizes());
		}

		static vk::UniqueDescriptorPool createDescriptorPool(const Graphics::Vulkan& vulkan){
			const std::array poolSizes = {
				vk::DescriptorPoolSize(
					vk::DescriptorType::eUniformBuffer,					//Descriptor type
					getUniformBufferSizes().size()						//Descriptor count
				)
			};

			const vk::DescriptorPoolCreateInfo createInfo(
				{},														//Flags
				1,														//Descriptor set count
				poolSizes.size(), poolSizes.data()						//Pool sizes
			);

			return vulkan.createDescriptorPool(createInfo);
		}

		static vk::DescriptorSet createDescriptorSet(	const Graphics::Vulkan& vulkan,
														vk::DescriptorPool pool )
		{
			const auto layout = getDescriptorSetLayout(vulkan);
			return vulkan.allocateDescriptorSet(pool, layout).release();
		}

		static vk::PipelineLayout createPipelineLayout(	const Graphics::Vulkan& vulkan,
														vk::DescriptorSetLayout keyFrameDescriptorSetLayout,
														vk::DescriptorSetLayout fillFrameDescriptorSetLayout ) 
		{
			using Index = std::tuple<vk::DescriptorSetLayout, vk::DescriptorSetLayout>;
			static std::unordered_map<Index, const Utils::StaticId, Utils::Hasher<Index>> ids; 

			const Index index(keyFrameDescriptorSetLayout, fillFrameDescriptorSetLayout);
			const auto& id = ids[index]; //TODO make it thread safe

			auto result = vulkan.createPipelineLayout(id);
			if(!result) {
				const std::array layouts = {
					RendererBase::getDescriptorSetLayout(vulkan), 			//DESCRIPTOR_SET_RENDERER
					getDescriptorSetLayout(vulkan), 						//DESCRIPTOR_SET_KEYER
					keyFrameDescriptorSetLayout, 							//DESCRIPTOR_SET_KEYFRAME
					fillFrameDescriptorSetLayout 							//DESCRIPTOR_SET_FILLFRAME
				};

				const vk::PipelineLayoutCreateInfo createInfo(
					{},													//Flags
					layouts.size(), layouts.data(),						//Descriptor set layouts
					0, nullptr											//Push constants
				);

				result = vulkan.createPipelineLayout(id, createInfo);
			}

			return result;
		}

		static vk::Pipeline createPipeline(	const Graphics::Vulkan& vulkan,
											vk::PipelineLayout layout,
											vk::RenderPass renderPass,
											BlendingMode blendingMode,
											RenderingLayer renderingLayer,
											const FragmentConstants& fragmentConstants )
		{
			using Index = std::tuple<	vk::PipelineLayout,
										vk::RenderPass,
										BlendingMode,
										RenderingLayer,
										std::array<std::byte, sizeof(FragmentConstants)> >;
			static std::unordered_map<Index, const Utils::StaticId, Utils::Hasher<Index>> ids;

			//Create a index for gathering the id
			std::array<std::byte, sizeof(FragmentConstants)> constantData;
			std::memcpy(&constantData, &fragmentConstants, constantData.size());
			const Index index(
				layout,
				renderPass,
				blendingMode,
				renderingLayer,
				constantData
			);

			//Try to retrieve the result from cache
			const auto& id = ids[index];
			auto result = vulkan.createGraphicsPipeline(id);
			if(!result) {
				//No luck, create it
				static //So that its ptr can be used as an identifier
				#include <keyer_vert.h>
				const size_t vertId = reinterpret_cast<uintptr_t>(keyer_vert);
				static
				#include <keyer_frag.h>
				const size_t fragId = reinterpret_cast<uintptr_t>(keyer_frag);

				//Try to retrive modules from cache
				auto vertexShader = vulkan.createShaderModule(vertId);
				if(!vertexShader) {
					//Modules isn't in cache. Create it
					vertexShader = vulkan.createShaderModule(vertId, keyer_vert);
				}

				auto fragmentShader = vulkan.createShaderModule(fragId);
				if(!fragmentShader) {
					//Modules isn't in cache. Create it
					fragmentShader = vulkan.createShaderModule(fragId, keyer_frag);
				}

				assert(vertexShader);
				assert(fragmentShader);

				//Set the specialization constants
				const vk::SpecializationInfo fragmentSpecializationInfo(
					FRAGMENT_SPECIALIZATION_LAYOUT.size(), FRAGMENT_SPECIALIZATION_LAYOUT.data(),
					sizeof(fragmentConstants), &fragmentConstants
				);

				//Define the shader modules
				constexpr auto SHADER_ENTRY_POINT = "main";
				const std::array shaderStages = {
					vk::PipelineShaderStageCreateInfo(		
						{},												//Flags
						vk::ShaderStageFlagBits::eVertex,				//Shader type
						vertexShader,									//Shader handle
						SHADER_ENTRY_POINT,								//Shader entry point
						nullptr											//Specialization constants
					),							
					vk::PipelineShaderStageCreateInfo(		
						{},												//Flags
						vk::ShaderStageFlagBits::eFragment,				//Shader type
						fragmentShader,									//Shader handle
						SHADER_ENTRY_POINT, 							//Shader entry point
						&fragmentSpecializationInfo						//Specialization constants
					),						
				};

				constexpr std::array vertexBindings = {
					vk::VertexInputBindingDescription(
						VERTEX_BUFFER_BINDING,
						sizeof(Vertex),
						vk::VertexInputRate::eVertex
					)
				};

				constexpr std::array vertexAttributes = {
					vk::VertexInputAttributeDescription(
						VERTEX_LOCATION_POSITION,
						VERTEX_BUFFER_BINDING,
						vk::Format::eR32G32Sfloat,
						offsetof(Vertex, position)
					),
					vk::VertexInputAttributeDescription(
						VERTEX_LOCATION_TEXCOORD,
						VERTEX_BUFFER_BINDING,
						vk::Format::eR32G32Sfloat,
						offsetof(Vertex, texCoord)
					),
					vk::VertexInputAttributeDescription(
						VERTEX_LOCATION_KLM,
						VERTEX_BUFFER_BINDING,
						vk::Format::eR32G32B32Sfloat,
						offsetof(Vertex, klm)
					)
				};

				const vk::PipelineVertexInputStateCreateInfo vertexInput(
					{},
					vertexBindings.size(), vertexBindings.data(),		//Vertex bindings
					vertexAttributes.size(), vertexAttributes.data()	//Vertex attributes
				);

				constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
					{},													//Flags
					vk::PrimitiveTopology::eTriangleStrip,				//Topology
					true												//Restart enable
				);

				constexpr vk::PipelineViewportStateCreateInfo viewport(
					{},													//Flags
					1, nullptr,											//Viewports (dynamic)
					1, nullptr											//Scissors (dynamic)
				);

				constexpr vk::PipelineRasterizationStateCreateInfo rasterizer(
					{},													//Flags
					false, 												//Depth clamp enabled
					false,												//Rasterizer discard enable
					vk::PolygonMode::eFill,								//Polygon mode
					vk::CullModeFlagBits::eNone, 						//Cull faces
					vk::FrontFace::eClockwise,							//Front face direction
					false, 0.0f, 0.0f, 0.0f,							//Depth bias
					1.0f												//Line width
				);

				constexpr vk::PipelineMultisampleStateCreateInfo multisample(
					{},													//Flags
					vk::SampleCountFlagBits::e1,						//Sample count
					false, 1.0f,										//Sample shading enable, min sample shading
					nullptr,											//Sample mask
					false, false										//Alpha to coverage, alpha to 1 enable
				);

				const auto depthStencil = Graphics::getDepthStencilConfiguration(renderingLayer);

				const std::array colorBlendAttachments = {
					Graphics::getBlendingConfiguration(blendingMode)
				};

				const vk::PipelineColorBlendStateCreateInfo colorBlend(
					{},													//Flags
					false,												//Enable logic operation
					vk::LogicOp::eCopy,									//Logic operation
					colorBlendAttachments.size(), colorBlendAttachments.data() //Blend attachments
				);

				constexpr std::array dynamicStates = {
					vk::DynamicState::eViewport,
					vk::DynamicState::eScissor
				};

				const vk::PipelineDynamicStateCreateInfo dynamicState(
					{},													//Flags
					dynamicStates.size(), dynamicStates.data()			//Dynamic states
				);

				const vk::GraphicsPipelineCreateInfo createInfo(
					{},													//Flags
					shaderStages.size(), shaderStages.data(),			//Shader stages
					&vertexInput,										//Vertex input
					&inputAssembly,										//Vertex assembly
					nullptr,											//Tesselation
					&viewport,											//Viewports
					&rasterizer,										//Rasterizer
					&multisample,										//Multisampling
					&depthStencil,										//Depth / Stencil tests
					&colorBlend,										//Color blending
					&dynamicState,										//Dynamic states
					layout,												//Pipeline layout
					renderPass, 0,										//Renderpasses
					nullptr, 0											//Inherit
				);

				result = vulkan.createGraphicsPipeline(id, createInfo);
			}

			assert(result);
			return result;
		}

	};

	using Input = Signal::Input<Video>;
	using LastFrames = std::unordered_map<const RendererBase*, std::pair<Video, Video>>;

	std::reference_wrapper<Keyer>			owner;

	Input									keyIn;
	Input									fillIn;

	Math::Vec2f								size;
	std::vector<Shape>						crop;

	bool 									lumaKeyEnabled;
	bool									chromaKeyEnabled;
	bool									linearKeyEnabled;

	bool									lumaKeyInverted;
	float									lumaKeyMinThreshold;
	float									lumaKeyMaxThreshold;

	float									chromaKeyHue;
	float									chromaKeyHueThreshold;
	float									chromaKeyHueSmoothness;
	float									chromaKeySaturationThreshold;
	float									chromaKeySaturationSmoothness;
	float									chromaKeyValueThreshold;
	float									chromaKeyValueSmoothness;

	bool									linearKeyInverted;
	Keyer::LinearKeyChannel					linearKeyChannel;

	std::unique_ptr<Open>					opened;
	LastFrames								lastFrames;
	

	KeyerImpl(	Keyer& owner, 
				Math::Vec2f size )
		: owner(owner)
		, keyIn(owner, "keyIn")
		, fillIn(owner, "fillIn")

		, size(size)
		, crop(1) //We'll fill its contents later

		, lumaKeyEnabled(false)
		, chromaKeyEnabled(false)
		, linearKeyEnabled(true)

		, lumaKeyInverted(false)
		, lumaKeyMinThreshold(0.5f)
		, lumaKeyMaxThreshold(0.5f)

		, chromaKeyHue(120.0f) //Green
		, chromaKeyHueThreshold(30.0f) 
		, chromaKeyHueSmoothness(10.0f)
		, chromaKeySaturationThreshold(0.4f)
		, chromaKeySaturationSmoothness(0.1f)
		, chromaKeyValueThreshold(0.3f)
		, chromaKeyValueSmoothness(0.1f)

		, linearKeyInverted(false)
		, linearKeyChannel(Keyer::LinearKeyChannel::fillA)

	{
		//We'll set a rectangle as our default crop
		generateRectangle(crop.front(), size);
	}

	~KeyerImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<Keyer&>(base);
		keyIn.setLayout(base);
		fillIn.setLayout(base);
	}

	void open(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& keyer = static_cast<Keyer&>(base);
		assert(&owner.get() == &keyer);
		assert(!opened);

		if(keyer.getRenderPass()) {
			//Create in a unlocked environment
			if(lock) lock->unlock();
			auto newOpened = Utils::makeUnique<Open>(
					keyer.getInstance().getVulkan(),
					getSize(),
					keyer.getScalingMode()
			);

			//Set all the parameters
			newOpened->updateModelMatrixUniform(keyer.getTransform());
			newOpened->updateOpacityUniform(keyer.getOpacity());

			newOpened->setCrop(getCrop());

			newOpened->updateLumaKeyEnabledConstant(getLumaKeyEnabled());
			newOpened->updateLumaKeyThresholdUniform(
				getLumaKeyInverted(),
				getLumaKeyMinThreshold(),
				getLumaKeyMaxThreshold()
			);

			newOpened->updateChromaKeyEnabledConstant(getChromaKeyEnabled());
			newOpened->updateChromaKeyHueUniform(getChromaKeyHue());
			newOpened->updateChromaKeyHueThresholdUniform(getChromaKeyHueThreshold());
			newOpened->updateChromaKeyHueSmoothnessUniform(getChromaKeyHueSmoothness());
			newOpened->updateChromaKeySaturationThresholdUniform(getChromaKeySaturationThreshold());
			newOpened->updateChromaKeySaturationSmoothnessUniform(getChromaKeySaturationSmoothness());
			newOpened->updateChromaKeyValueThresholdUniform(getChromaKeyValueThreshold());
			newOpened->updateChromaKeyValueSmoothnessUniform(getChromaKeyValueSmoothness());

			newOpened->updateLinearKeyTypeConstant(
				getLinearKeyEnabled(),
				getLinearKeyInverted(),
				getLinearKeyChannel()
			);

			if(lock) lock->lock();

			//Write changes after locking back
			opened = std::move(newOpened);
		}

		assert(lastFrames.empty()); //Any hasChanged() should return true
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		open(base, &lock);
		assert(lock.owns_lock());
	}


	void close(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& keyer = static_cast<Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		//Write changes
		keyIn.reset();
		fillIn.reset();
		lastFrames.clear();
		auto oldOpened = std::move(opened);

		//Destroy the object in a unlocked environment
		if(oldOpened) {
			if(lock) lock->unlock();
			oldOpened.reset();
			if(lock) lock->lock();
		}

		assert(!opened);
	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	bool hasChangedCallback(const LayerBase& base, const RendererBase& renderer) const {
		const auto& keyer = static_cast<const Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		const auto ite = lastFrames.find(&renderer);
		if(ite == lastFrames.cend()) {
			//There is no frame previously rendered for this renderer
			return true;
		}

		if(	ite->second.first != keyIn.getLastElement() ||
			ite->second.second != fillIn.getLastElement() )
		{
			//A new frame has arrived since the last rendered one at this renderer
			return true;
		}

		if(keyIn.hasChanged() || fillIn.hasChanged()) {
			//A new frame is available
			return true;
		}

		//Nothing has changed :-)
		return false;
	}

	bool hasAlphaCallback(const LayerBase& base) const noexcept {
		const auto& keyer = static_cast<const Keyer&>(base);
		assert(&owner.get() == &keyer); Utils::ignore(keyer);

		bool result;

		//HACK using last element instead of pull for optimization reasons.
		//If changing from a alpha-less format to a alpha-ed format, a frame
		//with potential incorrect ordering will be rendered once. 
		const auto& lastElement = fillIn.getLastElement();

		if(lastElement) {
			if(getLumaKeyEnabled()) {
				result = true;
			} else if(getChromaKeyEnabled()) {
				result = true;
			} else if(getLinearKeyEnabled()) {
				if(getLinearKeyChannel() == Keyer::LinearKeyChannel::fillA) {
					result = Zuazo::hasAlpha(lastElement->getDescriptor()->getColorFormat());
				} else {
					result = true;
				}
			} else {
				//Default to no
				result = false;
			}
		} else {
			//No frame. Nothing will be rendered
			result = false;
		}

		return result;
	}

	void drawCallback(const LayerBase& base, const RendererBase& renderer, Graphics::CommandBuffer& cmd) {
		const auto& keyer = static_cast<const Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		if(opened) {
			const auto& keyFrame = keyIn.pull();
			const auto& fillFrame = fillIn.pull();
			
			//Draw
			if(keyFrame && fillFrame) {
				opened->draw(
					cmd, 
					keyFrame, 
					fillFrame,
					keyer.getScalingFilter(),
					keyer.getRenderPass(),
					keyer.getBlendingMode(),
					keyer.getRenderingLayer()
				);
			}

			//Update the state for next hasChanged()
			lastFrames[&renderer] = std::make_pair(keyFrame, fillFrame);
		}
	}

	void transformCallback(LayerBase& base, const Math::Transformf& transform) {
		auto& keyer = static_cast<Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		if(opened) {
			opened->updateModelMatrixUniform(transform);
		}

		lastFrames.clear(); //Will force hasChanged() to true
	}

	void opacityCallback(LayerBase& base, float opa) {
		auto& keyer = static_cast<Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		if(opened) {
			opened->updateOpacityUniform(opa);
		}

		lastFrames.clear(); //Will force hasChanged() to true
	}

	void blendingModeCallback(LayerBase& base, BlendingMode mode) {
		auto& keyer = static_cast<Keyer&>(base);
		recreateCallback(keyer, keyer.getRenderPass(), mode);
	}

	void renderingLayerCallback(LayerBase& base, RenderingLayer) {
		auto& keyer = static_cast<Keyer&>(base);
		recreateCallback(keyer, keyer.getRenderPass(), keyer.getBlendingMode());
	}

	void renderPassCallback(LayerBase& base, vk::RenderPass renderPass) {
		auto& keyer = static_cast<Keyer&>(base);
		recreateCallback(keyer, renderPass, keyer.getBlendingMode());
	}

	void scalingModeCallback(VideoScalerBase& base, ScalingMode mode) {
		auto& keyer = static_cast<Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		if(opened) {
			opened->frameGeometry.setScalingMode(mode);
		}

		lastFrames.clear(); //Will force hasChanged() to true
	}

	void scalingFilterCallback(VideoScalerBase& base, ScalingFilter) {
		auto& keyer = static_cast<Keyer&>(base);
		assert(&owner.get() == &keyer); (void)(keyer);

		lastFrames.clear(); //Will force hasChanged() to true
	}



	void setSize(Math::Vec2f size) {
		if(this->size != size) {
			this->size = size;

			if(opened) {
				opened->frameGeometry.setTargetSize(this->size);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	Math::Vec2f getSize() const {
		return size;
	}


	void setCrop(Utils::BufferView<const Shape> shapes) {
		this->crop.clear();
		this->crop.insert(this->crop.cend(), shapes.cbegin(), shapes.cend());

		if(opened) {
			opened->setCrop(this->crop);
		}

		lastFrames.clear(); //Will force hasChanged() to true
	}

	Utils::BufferView<const Shape> getCrop() const {
		return crop;
	}
	


	void setLumaKeyEnabled(bool ena) {
		if(lumaKeyEnabled != ena) {
			lumaKeyEnabled = ena;

			if(opened) {
				opened->updateLumaKeyEnabledConstant(lumaKeyEnabled);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	bool getLumaKeyEnabled() const noexcept {
		return lumaKeyEnabled;
	}


	void setLumaKeyInverted(bool inv) {
		if(lumaKeyInverted != inv) {
			lumaKeyInverted = inv;

			if(opened) {
				opened->updateLumaKeyThresholdUniform(
					lumaKeyInverted, 
					lumaKeyMinThreshold, 
					lumaKeyMaxThreshold
				);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	bool getLumaKeyInverted() const noexcept {
		return lumaKeyInverted;
	}


	void setLumaKeyMinThreshold(float threshold) {
		if(lumaKeyMinThreshold != threshold) {
			lumaKeyMinThreshold = threshold;

			if(opened) {
				opened->updateLumaKeyThresholdUniform(
					lumaKeyInverted, 
					lumaKeyMinThreshold, 
					lumaKeyMaxThreshold
				);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getLumaKeyMinThreshold() const noexcept {
		return lumaKeyMinThreshold;
	}


	void setLumaKeyMaxThreshold(float threshold) {
		if(lumaKeyMaxThreshold != threshold) {
			lumaKeyMaxThreshold = threshold;

			if(opened) {
				opened->updateLumaKeyThresholdUniform(
					lumaKeyInverted, 
					lumaKeyMinThreshold, 
					lumaKeyMaxThreshold
				);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getLumaKeyMaxThreshold() const noexcept {
		return lumaKeyMaxThreshold;
	}



	void setChromaKeyEnabled(bool ena) {
		if(chromaKeyEnabled != ena) {
			chromaKeyEnabled = ena;

			if(opened) {
				opened->updateChromaKeyEnabledConstant(chromaKeyEnabled);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	bool getChromaKeyEnabled() const noexcept {
		return chromaKeyEnabled;
	}


	void setChromaKeyHue(float hue) {
		if(chromaKeyHue != hue) {
			chromaKeyHue = hue;

			if(opened) {
				opened->updateChromaKeyHueUniform(chromaKeyHue);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeyHue() const noexcept {
		return chromaKeyHue;
	}


	void setChromaKeyHueThreshold(float threshold) {
		if(chromaKeyHueThreshold != threshold) {
			chromaKeyHueThreshold = threshold;

			if(opened) {
				opened->updateChromaKeyHueThresholdUniform(chromaKeyHueThreshold);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeyHueThreshold() const noexcept {
		return chromaKeyHueThreshold;
	}


	void setChromaKeyHueSmoothness(float smoothness) {
		if(chromaKeyHueSmoothness != smoothness) {
			chromaKeyHueSmoothness = smoothness;

			if(opened) {
				opened->updateChromaKeyHueSmoothnessUniform(chromaKeyHueSmoothness);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeyHueSmoothness() const noexcept {
		return chromaKeyHueSmoothness;
	}


	void setChromaKeySaturationThreshold(float threshold) {
		if(chromaKeySaturationThreshold != threshold) {
			chromaKeySaturationThreshold = threshold;

			if(opened) {
				opened->updateChromaKeySaturationThresholdUniform(chromaKeySaturationThreshold);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeySaturationThreshold() const noexcept {
		return chromaKeySaturationThreshold;
	}


	void setChromaKeySaturationSmoothness(float smoothness) {
		if(chromaKeySaturationSmoothness != smoothness) {
			chromaKeySaturationSmoothness = smoothness;

			if(opened) {
				opened->updateChromaKeySaturationSmoothnessUniform(chromaKeySaturationSmoothness);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeySaturationSmoothness() const noexcept {
		return chromaKeySaturationSmoothness;
	}


	void setChromaKeyValueThreshold(float threshold) {
		if(chromaKeyValueThreshold != threshold) {
			chromaKeyValueThreshold = threshold;

			if(opened) {
				opened->updateChromaKeyValueThresholdUniform(chromaKeyValueThreshold);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeyValueThreshold() const noexcept {
		return chromaKeyValueThreshold;
	}


	void setChromaKeyValueSmoothness(float smoothness) {
		if(chromaKeyValueSmoothness != smoothness) {
			chromaKeyValueSmoothness = smoothness;

			if(opened) {
				opened->updateChromaKeyValueSmoothnessUniform(chromaKeyValueSmoothness);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	float getChromaKeyValueSmoothness() const noexcept {
		return chromaKeyValueSmoothness;
	}



	void setLinearKeyEnabled(bool ena) {
		if(linearKeyEnabled != ena) {
			linearKeyEnabled = ena;

			if(opened) {
				opened->updateLinearKeyTypeConstant(
					linearKeyEnabled,
					linearKeyInverted,
					linearKeyChannel
				);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	bool getLinearKeyEnabled() const noexcept {
		return linearKeyEnabled;
	}


	void setLinearKeyInverted(bool inv) {
		if(linearKeyInverted != inv) {
			linearKeyInverted = inv;

			if(opened) {
				opened->updateLinearKeyTypeConstant(
					linearKeyEnabled,
					linearKeyInverted,
					linearKeyChannel
				);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	bool getLinearKeyInverted() const noexcept {
		return linearKeyInverted;
	}


	void setLinearKeyChannel(Keyer::LinearKeyChannel ch) {
		if(linearKeyChannel != ch) {
			linearKeyChannel = ch;

			if(opened) {
				opened->updateLinearKeyTypeConstant(
					linearKeyEnabled,
					linearKeyInverted,
					linearKeyChannel
				);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

	Keyer::LinearKeyChannel getLinearKeyChannel() const noexcept {
		return linearKeyChannel;
	}
		

private:
	void recreateCallback(	Keyer& keyer, 
							vk::RenderPass renderPass,
							BlendingMode blendingMode )
	{
		assert(&owner.get() == &keyer);

		if(keyer.isOpen()) {
			const bool isValid = 	renderPass &&
									blendingMode > BlendingMode::none ;

			if(opened && isValid) {
				//It remains valid
				opened->recreate();
			} else if(opened && !isValid) {
				//It has become invalid
				keyIn.reset();
				fillIn.reset();
				opened.reset();
			} else if(!opened && isValid) {
				//It has become valid
				open(keyer, nullptr);
			}

			lastFrames.clear(); //Will force hasChanged() to true
		}
	}

};




Keyer::Keyer(	Instance& instance,
				std::string name,
				Math::Vec2f size )
	: Utils::Pimpl<KeyerImpl>({}, *this, size)
	, Base(
		instance, 
		std::move(name),
		{ PadRef((*this)->keyIn), PadRef((*this)->fillIn) },
		std::bind(&KeyerImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&KeyerImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&KeyerImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&KeyerImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		{},
		std::bind(&KeyerImpl::transformCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::opacityCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::blendingModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::renderingLayerCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::hasChangedCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::hasAlphaCallback, std::ref(**this), std::placeholders::_1),
		std::bind(&KeyerImpl::drawCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(&KeyerImpl::renderPassCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, VideoScalerBase(
		std::bind(&KeyerImpl::scalingModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&KeyerImpl::scalingFilterCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
}

Keyer::Keyer(Keyer&& other) = default;

Keyer::~Keyer() = default;

Keyer& Keyer::operator=(Keyer&& other) = default;


void Keyer::setSize(Math::Vec2f size) {
	(*this)->setSize(size);
}

Math::Vec2f Keyer::getSize() const noexcept {
	return (*this)->getSize();
}


void Keyer::setCrop(Utils::BufferView<const Shape> shapes) {
	(*this)->setCrop(shapes);
}

Utils::BufferView<const Shape> Keyer::getCrop() const noexcept {
	return (*this)->getCrop();
}



void Keyer::setLumaKeyEnabled(bool ena) {
	(*this)->setLumaKeyEnabled(ena);
}


bool Keyer::getLumaKeyEnabled() const noexcept {
	return (*this)->getLumaKeyEnabled();
}


void Keyer::setLumaKeyInverted(bool inv) {
	(*this)->setLumaKeyInverted(inv);
}

bool Keyer::getLumaKeyInverted() const noexcept {
	return (*this)->getLumaKeyInverted();
}


void Keyer::setLumaKeyMinThreshold(float threshold) {
	(*this)->setLumaKeyMinThreshold(threshold);
}

float Keyer::getLumaKeyMinThreshold() const noexcept {
	return (*this)->getLumaKeyMinThreshold();
}


void Keyer::setLumaKeyMaxThreshold(float threshold) {
	(*this)->setLumaKeyMaxThreshold(threshold);
}

float Keyer::getLumaKeyMaxThreshold() const noexcept {
	return (*this)->getLumaKeyMaxThreshold();
}



void Keyer::setChromaKeyEnabled(bool ena) {
	(*this)->setChromaKeyEnabled(ena);
}

bool Keyer::getChromaKeyEnabled() const noexcept {
	return (*this)->getChromaKeyEnabled();
}


void Keyer::setChromaKeyHue(float hue) {
	(*this)->setChromaKeyHue(hue);
}

float Keyer::getChromaKeyHue() const noexcept {
	return (*this)->getChromaKeyHue();
}


void Keyer::setChromaKeyHueThreshold(float threshold) {
	(*this)->setChromaKeyHueThreshold(threshold);
}

float Keyer::getChromaKeyHueThreshold() const noexcept {
	return (*this)->getChromaKeyHueThreshold();
}


void Keyer::setChromaKeyHueSmoothness(float smoothness) {
	(*this)->setChromaKeyHueSmoothness(smoothness);
}

float Keyer::getChromaKeyHueSmoothness() const noexcept {
	return (*this)->getChromaKeyHueSmoothness();
}


void Keyer::setChromaKeySaturationThreshold(float threshold) {
	(*this)->setChromaKeySaturationThreshold(threshold);
}

float Keyer::getChromaKeySaturationThreshold() const noexcept {
	return (*this)->getChromaKeySaturationThreshold();
}


void Keyer::setChromaKeySaturationSmoothness(float smoothness) {
	(*this)->setChromaKeySaturationSmoothness(smoothness);
}

float Keyer::getChromaKeySaturationSmoothness() const noexcept {
	return (*this)->getChromaKeySaturationSmoothness();
}


void Keyer::setChromaKeyValueThreshold(float threshold) {
	(*this)->setChromaKeyValueThreshold(threshold);
}

float Keyer::getChromaKeyValueThreshold() const noexcept {
	return (*this)->getChromaKeyValueThreshold();
}


void Keyer::setChromaKeyValueSmoothness(float smoothness) {
	(*this)->setChromaKeyValueSmoothness(smoothness);
}

float Keyer::getChromaKeyValueSmoothness() const noexcept {
	return (*this)->getChromaKeyValueSmoothness();
}



void Keyer::setLinearKeyEnabled(bool ena) {
	(*this)->setLinearKeyEnabled(ena);
}

bool Keyer::getLinearKeyEnabled() const noexcept {
	return (*this)->getLinearKeyEnabled();
}


void Keyer::setLinearKeyInverted(bool inv) {
	(*this)->setLinearKeyInverted(inv);
}

bool Keyer::getLinearKeyInverted() const noexcept {
	return (*this)->getLinearKeyInverted();
}


void Keyer::setLinearKeyChannel(LinearKeyChannel ch) {
	(*this)->setLinearKeyChannel(ch);
}

Keyer::LinearKeyChannel Keyer::getLinearKeyChannel() const noexcept {
	return (*this)->getLinearKeyChannel();
}

}