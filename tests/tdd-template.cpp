#include <iostream>

template <typename Derived>
class package {
public:
    void execute() {
        static_cast<Derived*>(this)->required_method();
    }

    void toString() {
        static_cast<Derived*>(this)->toStringImpl();
    }
};

template <typename Derived>
class Request : public package<Derived> {  // 继承package<Derived>
public:
    void required_method() {
        std::cout << "Request processing\n";
    }

    void serialize() {
        static_cast<Derived*>(this)->serialize_impl();
    }

    // 提供基类的toStringImpl实现
    void toStringImpl() {
        std::cout << "Request toStringImpl\n";
    }
};

template <typename Derived>
class Response : public package<Response<Derived>> {
public:
    void required_method() {
        std::cout << "Response processing\n";
    }

    void deserialize() {
        static_cast<Derived*>(this)->deserialize_impl();
    }
};

class MyRequest : public Request<MyRequest> {
public:
    void serialize_impl() {
        std::cout << "MyRequest serialization\n";
    }

    // 覆盖toStringImpl
    void toStringImpl() {
        std::cout << "MyRequest toStringImpl\n";
        // 调用基类实现
        //Request<MyRequest>::toStringImpl();
    }
};

class MyResponse : public Response<MyResponse> {
public:
    void deserialize_impl() {
        std::cout << "MyResponse deserialization\n";
    }
};

int main() {
    MyRequest req;
    req.execute();  // Request processing
    req.serialize(); // MyRequest serialization

    std::cout << "1" << std::endl;
    // 调用派生类实现
    req.toString(); // MyRequest toStringImpl + Request toStringImpl
    std::cout << "2" << std::endl;
    // 显式调用基类实现
    req.toStringImpl(); // Request toStringImpl
    std::cout << "3" << std::endl;
    req.Request::toStringImpl();
    std::cout << "4" << std::endl;
    MyResponse resp;
    resp.execute(); // Response processing
    resp.deserialize(); // MyResponse deserialization

    return 0;
}