#ifndef IRIS_COMPONENT_HPP
#define IRIS_COMPONENT_HPP

namespace iris::core {
    class Component{
    public:
        virtual ~Component() = default;

        virtual void update() = 0;
    };
}


#endif //IRIS_COMPONENT_HPP
