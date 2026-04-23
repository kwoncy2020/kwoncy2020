export module ControllerFactory;

import ImageEditController;
import ImageDataRepository;

export class ControllerFactory {
public:
    static ImageEditController* CreateController();
};
