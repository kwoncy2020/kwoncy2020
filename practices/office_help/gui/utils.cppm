module;
#include <string>
#include <tuple>

export module Utils;


export class Utils{
public:
    static std::tuple<int,int,int,int> getBoxValuesFromPoints(int x1, int y1, int x2, int y2, std::string convertS = "xywh") {
        if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) {
            // throw std::invalid_argument("Coordinates must be positive");
            return std::make_tuple(0, 0, 0, 0);
        }
        int startX, startY, endX, endY;
        
        if (x1 > x2){
            startX = x2;
            endX = x1;
        } else {
            startX = x1;
            endX = x2;
        }
        if (y1 > y2){
            startY = y2;
            endY = y1;
        } else {
            startY = y1;
            endY = y2;
        }
        
        if (convertS == "xywh") {
            return std::make_tuple(startX, startY, endX-startX, endY-startY);
        } else if (convertS == "xyxy"){
            return std::make_tuple(startX, startY, endX, endY);
        }

        return std::make_tuple(0, 0, 0, 0);
    }
};
