

module ControllerFactory;

import ImageEditController;
import ImageDataRepository;

ImageEditController* ControllerFactory::CreateController() {
    static ImageDataRepository* repository = &ImageDataRepository::GetInstance();
    static ImageEditController* controller = new ImageEditController(repository);
    return controller;
}