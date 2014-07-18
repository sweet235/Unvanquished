// THIS FILE IS AUTO GENERATED, EDIT AT YOUR OWN RISK

#include <tuple>

// Implementation of the base entity class

// Constructor of entity
Entity::Entity(MessageHandler *messageHandlers, const int* componentOffsets): messageHandlers(messageHandlers), componentOffsets(componentOffsets) {
}

void Entity::SendMessage(int msg, const void* data) {
    MessageHandler handler = messageHandlers[msg];
    if (handler) {
        handler(this, data);
    }
}

// Entity helper functions to send message e.g.
//   void Entity::Damage(int value);
{% for message in messages %}
    void Entity::{{message.name}}({{message.get_function_args()}}) {
        {% if message.get_num_args() == 0 %}
            SendMessage({{message.get_enum_name()}}, nullptr);
        {% else %}
            {{message.get_tuple_type()}} data({{message.get_args_names()}});
            SendMessage({{message.get_enum_name()}}, &data);
        {% endif %}
    }
{% endfor %}

// Entity helper functions to get the components e.g.
//   HealthComponent* GetHealthComponent();
{% for component in components %}
    {{component.get_type_name()}}* Entity::Get{{component.get_type_name()}}() {
        int index = {{component.get_priority()}};
        int offset = componentOffsets[index];
        if (offset) {
            return ({{component.get_type_name()}}*) (((char*) this) + offset);
        } else {
            return nullptr;
        }
    }
{% endfor %}

// Implementation of the components

// Component helper functions to change the attributes values e.g.
//   void SetHealth(int value);
{% for component in components %}
    {% for attrib in component.get_own_attribs() %}
        void {{component.get_base_type_name()}}::{{attrib.get_setter_name()}}({{attrib.typ}} value) {
            entity->SendMessage({{attrib.get_message().get_enum_name()}}, new {{attrib.typ}}(value));
        }
    {% endfor %}
{% endfor %}

// Implementation of the entities


{% for entity in entities %}
    // The vtable of offset of components in an entity
    // TODO: doesn't handle component inheritance?
    static int {{entity.get_type_name()}}componentOffsets[] = {
        {% for component in components %}
            {% if component in entity.get_components() %}
                offsetof({{entity.get_type_name()}}, {{component.get_variable_name()}}),
            {% else %}
                0,
            {% endif %}
        {% endfor %}
    };

    // The static message handlers put in the vtable
    {% for message in entity.get_messages_to_handle() %}
        void {{entity.get_message_handler_name(message)}}(Entity* _entity, const void* _data) {
            // Cast the entity to the correct type (receive an Entity*)
            {{entity.get_type_name()}}* entity = ({{entity.get_type_name()}}*) _entity;

            {% if message.get_num_args == 0 %}
                // No argument for the message, just call the handlers of all the components
                {% for component in entity.get_components() %}
                    {% if message in component.get_messages_to_handle() %}
                        entity->{{component.get_variable_name()}}->{{message.get_handler_name()}}();
                    {% endif %}
                {% endfor %}
            {% else %}
                // Cast the message content to the right type (receive a const void*)
                const {{message.get_tuple_type()}}* data = (const {{message.get_tuple_type()}}*) _data;
                {% for component in entity.get_components() %}
                    {% if message in component.get_messages_to_handle() %}
                        entity->{{component.get_variable_name()}}->{{message.get_handler_name()}}({{message.get_unpacked_tuple_args('*data')}});
                    {% endif %}
                {% endfor %}
            {% endif %}
        }
    {% endfor%}

    // The vtable of message handlers for an entity
    static MessageHandler {{entity.get_type_name()}}messageHandlers[] = {
        {% for message in messages %}
            {% if message in entity.get_messages_to_handle() %}
                {{entity.get_message_handler_name(message)}},
            {% else %}
                nullptr,
            {% endif %}
        {% endfor%}
    };

    // Fat constructor for the entity that initializes the components.
    {{entity.get_type_name()}}::{{entity.get_type_name()}}(): Entity(messageHandlers, componentOffsets)
    //TODO make sure it is in order
    {% for component in entity.get_components() %}
        // Each component takes the entity it is in, its parameters, the shared attributes and the components it requires
        , {{component.get_variable_name()}}(new {{component.get_type_name()}}(
            this
            {% for param in component.get_param_names() %}
                , {{entity.get_params()[component.name][param]}}
            {% endfor %}
            {% for attrib in component.get_attribs() %}
                , {{attrib.get_variable_name()}}
            {% endfor %}
            {% for required in component.get_required_components() %}
                , {{required.get_variable_name()}}
            {% endfor %}
        ))
    {% endfor %}
    {}

{% endfor %}