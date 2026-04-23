#include <iostream>
#include <opencv2/opencv.hpp>

// Import modules
import ImageData;
import ImageDataRepository;
import ImageController;

int main() {
    std::cout << "Testing ImageController..." << std::endl;
    
    // Get repository instance
    auto& repository = ImageDataRepository::GetInstance();
    
    // Create controller with repository
    ImageController controller(&repository);
    
    // Test 1: Load image from file (create a simple test image first)
    cv::Mat testImage = cv::Mat::zeros(100, 100, CV_8UC3);
    cv::rectangle(testImage, cv::Rect(10, 10, 80, 80), cv::Scalar(255, 0, 0), -1);
    
    // Test 2: Load image from matrix
    std::string imageId = controller.LoadImageFromMat(testImage, "test_image");
    if (imageId.empty()) {
        std::cout << "FAILED: Could not load image from matrix" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Loaded image with ID: " << imageId << std::endl;
    
    // Test 3: Check if image exists
    if (!controller.HasImage(imageId)) {
        std::cout << "FAILED: Image not found after loading" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Image found in controller" << std::endl;
    
    // Test 4: Get image name
    std::string imageName = controller.GetImageName(imageId);
    if (imageName != "test_image") {
        std::cout << "FAILED: Wrong image name, got: " << imageName << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Correct image name: " << imageName << std::endl;
    
    // Test 5: Get image data
    ImageData* imageData = controller.GetImageData(imageId);
    if (!imageData || !imageData->HasImage()) {
        std::cout << "FAILED: Could not get image data" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Image data retrieved, size: " 
              << imageData->GetWidth() << "x" << imageData->GetHeight() << std::endl;
    
    // Test 6: Add bounding box
    BoundingBox box(20, 20, 30, 30);
    controller.AddBoundingBox(imageId, box);
    
    auto boxes = controller.GetBoundingBoxes(imageId);
    if (boxes.size() != 1) {
        std::cout << "FAILED: Could not add bounding box" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Added bounding box, count: " << boxes.size() << std::endl;
    
    // Test 7: Get image count
    size_t count = controller.GetImageCount();
    if (count != 1) {
        std::cout << "FAILED: Wrong image count: " << count << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Correct image count: " << count << std::endl;
    
    // Test 8: Check metadata
    ImageData* imageData = controller.GetImageData(imageId);
    if (!imageData) {
        std::cout << "FAILED: Could not get image data for metadata test" << std::endl;
        return 1;
    }
    
    // Test image metadata
    int channels = imageData->GetChannels();
    int depth = imageData->GetDepth();
    size_t imageSize = imageData->GetImageSize();
    std::string format = imageData->GetImageFormat();
    
    std::cout << "SUCCESS: Image metadata - Channels: " << channels 
              << ", Depth: " << depth 
              << ", Size: " << imageSize << " bytes"
              << ", Format: " << format << std::endl;
    
    // Test 9: Remove image
    if (!controller.RemoveImage(imageId)) {
        std::cout << "FAILED: Could not remove image" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Image removed" << std::endl;
    
    // Test 10: Verify removal
    if (controller.HasImage(imageId)) {
        std::cout << "FAILED: Image still exists after removal" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Image properly removed" << std::endl;
    
    std::cout << "\nAll tests PASSED! ImageController with metadata is working correctly." << std::endl;
    return 0;
}
