#pragma once

#include "../term/builder_i.hpp"

namespace flecs 
{

// Filter builder interface
template<typename Base, typename ... Components>
struct filter_builder_i : term_builder_i<Base> {
    filter_builder_i(ecs_filter_desc_t *desc, int32_t term_index = 0) 
        : m_term_index(term_index)
        , m_desc(desc) { }

    Base& instanced() {
        m_desc->instanced = true;
        return *this;
    }

    Base& expr(const char *expr) {
        m_desc->expr = expr;
        return *this;
    }

    Base& term() {
        if (m_term_index >= ECS_TERM_DESC_CACHE_SIZE) {
            if (m_term_index == ECS_TERM_DESC_CACHE_SIZE) {
                m_desc->terms_buffer = ecs_os_calloc_n(
                    ecs_term_t, m_term_index + 1);
                ecs_os_memcpy_n(m_desc->terms_buffer, m_desc->terms, 
                    ecs_term_t, m_term_index);
                ecs_os_memset_n(m_desc->terms, 0, 
                    ecs_term_t, ECS_TERM_DESC_CACHE_SIZE);
            } else {
                m_desc->terms_buffer = ecs_os_realloc_n(m_desc->terms_buffer, 
                    ecs_term_t, m_term_index + 1);
            }

            m_desc->terms_buffer_count = m_term_index + 1;

            this->set_term(&m_desc->terms_buffer[m_term_index]);
        } else {
            this->set_term(&m_desc->terms[m_term_index]);
        }

        m_term_index ++;
        return *this;
    }
    
    Base& arg(int32_t term_index) {
        ecs_assert(term_index > 0, ECS_INVALID_PARAMETER, NULL);
        int32_t prev_index = m_term_index;
        m_term_index = term_index - 1;
        this->term();
        m_term_index = prev_index;
        ecs_assert(ecs_term_is_initialized(this->m_term), ECS_INVALID_PARAMETER, NULL);
        return *this;
    }    

    template<typename T>
    Base& term() {
        this->term();
        *this->m_term = flecs::term(this->world_v()).id<T>().move();
        this->m_term->inout = static_cast<ecs_inout_kind_t>(_::type_to_inout<T>());
        return *this;
    }

    Base& term(id_t id) {
        this->term();
        *this->m_term = flecs::term(id).move();
        return *this;
    }

    template<typename R, typename O>
    Base& term() {
        this->term();
        *this->m_term = flecs::term(this->world_v()).id<R, O>().move();
        return *this;
    }

    template<typename R>
    Base& term(id_t o) {
        this->term();
        *this->m_term = flecs::term(this->world_v()).id<R>(o).move();
        return *this;
    }     

    Base& term(id_t r, id_t o) {
        this->term();
        *this->m_term = flecs::term(this->world_v()).id(r, o).move();
        return *this;
    }

    Base& term(const flecs::type& type) {
        this->term();
        *this->m_term = flecs::term(this->world_v()).id(type).move();
        return *this;
    }

    Base& term(const flecs::type& type, flecs::id_t o) {
        this->term();
        *this->m_term = flecs::term(this->world_v()).id(type, o).move();
        return *this;
    }

    Base& term(const char *expr) {
        this->term();
        *this->m_term = flecs::term(this->world_v()).expr(expr).move();
        return *this;
    }

    Base& term(flecs::term& term) {
        this->term();
        *this->m_term = term.move();
        return *this;
    }

    Base& term(flecs::term&& term) {
        this->term();
        *this->m_term = term.move();
        return *this;
    }

protected:
    virtual flecs::world_t* world_v() = 0;
    int32_t m_term_index;

private:
    operator Base&() {
        return *static_cast<Base*>(this);
    }

    ecs_filter_desc_t *m_desc;
};

}
