#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

    class Builder;
    class KeyContext;
    class ValueKeyContext;
    class StartDictContext;
    class StartArrayContext;

    class BaseContext {
    public:  
        BaseContext(Builder& builder)
            : builder_(builder) {
        }
    protected:
        Builder& builder_;
    };

    class StartContainersContext : public BaseContext {
    public:
        StartContainersContext(Builder& builder) 
            : BaseContext(builder) {
        }
        StartArrayContext& StartArray();
        StartDictContext& StartDict();
    };

    class KeyContext : public StartContainersContext {        
    public:
        KeyContext(Builder& builder) 
            : StartContainersContext(builder) {
        }
        ValueKeyContext Value(Node::Value value);
    };

    class ValueKeyContext : public BaseContext {
    public:
        ValueKeyContext(Builder& builder)
            : BaseContext(builder) {
        }
        KeyContext& Key(std::string key);
        Builder& EndDict();
    };

    class StartDictContext : public BaseContext {
    public:
        StartDictContext(Builder& builder)
            : BaseContext(builder) {
        }
        KeyContext& Key(std::string key);
        Builder& EndDict();
    };

    class StartArrayContext : public StartContainersContext {      
    public:
        StartArrayContext(Builder& builder)
            : StartContainersContext(builder) {
        }
        StartArrayContext& Value(Node::Value value);
        Builder& EndArray();
    };

    class Builder : virtual public KeyContext,
                    virtual public ValueKeyContext,
                    virtual public StartDictContext,
                    virtual public StartArrayContext {
    public:
        Builder();
        Node Build();
        KeyContext& Key(std::string key);
        Builder& Value(Node::Value value);
        StartDictContext& StartDict();
        StartArrayContext& StartArray();
        Builder& EndDict();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;
    
        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);
    };

}  // namespace json