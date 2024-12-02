/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _LINKED_LIST_HPP
#define _LINKED_LIST_HPP

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <functional>

namespace LinkedList {

    struct Node {
        Node* previous;
        uint64_t data;
        Node* next;
    };

        // Get length of the Linked list
    uint64_t length(Node* head);

        // Helper function that allocates a new node with the given data and NULL previous and next pointers.
    Node* newNode(uint64_t data);

        // Recursive function to insert a node in the list with given data and returns the head node
    void insertNode(Node*& head, uint64_t data);

        // Get a pointer to a node from its data
    Node* findNode(Node* head, uint64_t data);

        // Delete a node
    void deleteNode(Node*& head, uint64_t key);

        // Delete a specific node
    void deleteNode(Node*& head, Node* node);

        // print the Linked list
    void fprint(FILE* file, Node* head);

    template<typename T>
    class SimpleLinkedList {
       public:
        SimpleLinkedList()
            : m_count(0), m_start(nullptr) {}
        ~SimpleLinkedList() {
            for (uint64_t i = 0; i < m_count; i++)
                remove((uint64_t)0);
        }

        void insert(const T* obj) {
            if (findNode(m_start, (uint64_t)&obj) != nullptr) {
                return; // object already exists
            }
            insertNode(m_start, (uint64_t)obj);
            m_count++;
        }
        T* get(uint64_t index) const {
            if (index >= m_count)
                return nullptr;
            Node* temp = m_start;
            for (uint64_t i = 0; i < index; i++) {
                if (temp == nullptr)
                    return nullptr;
                temp = temp->next;
            }
            if (temp == nullptr)
                return nullptr;
            return (T*)(temp->data);
        }
        uint64_t getIndex(const T* obj) const {
            Node* temp = m_start;
            for (uint64_t i = 0; i < m_count; i++) {
                if (temp == nullptr)
                    return UINT64_MAX;
                if (temp->data == (uint64_t)obj)
                    return i;
                temp = temp->next;
            }
            return UINT64_MAX;
        }
        void remove(uint64_t index) {
            deleteNode(m_start, (uint64_t)get(index));
            m_count--;
        }
        void remove(const T* obj) {
            deleteNode(m_start, (uint64_t)obj);
            m_count--;
        }
        void rotateLeft() {
            if (m_count < 2) {
                return; // not enough nodes to rotate
            }
            Node* end = m_start;
            assert(end != nullptr);
            while (end->next != nullptr)
                end = end->next;
            assert(end != nullptr);
            end->next = m_start;
            m_start->previous = end;
            m_start = m_start->next;
            m_start->previous = nullptr;
            end->next->next = nullptr;
        }
        void rotateRight() {
            if (m_count < 2)
                return; // not enough nodes to rotate
            Node* end = m_start;
            assert(end != nullptr);
            while (end->next != nullptr)
                end = end->next;
            assert(end != nullptr);
            m_start->previous = end;
            end->next = m_start;
            end->previous = nullptr;
            m_start = end;
        }
        T* getHead() {
            if (m_start == nullptr)
                return nullptr;
            return (T*)(m_start->data);
        }
        void fprint(FILE* file) {
            fprintf(file, "LinkedList order: ");
            Node* node = m_start;
            for (uint64_t i = 0; i < m_count; i++) {
                if (node == nullptr)
                    break;
                fprintf(file, "%lx ", node->data);
                node = node->next;
            }
            fprintf(file, "\n");
        }

        uint64_t getCount() const {
            return m_count;
        }

       private:
        uint64_t m_count;
        Node* m_start;
    };

    template<typename T>
    class RearInsertLinkedList {
       public:
        RearInsertLinkedList()
            : m_count(0), m_start(nullptr), m_end(nullptr) {}
        ~RearInsertLinkedList() {
            for (uint64_t i = 0; i < m_count; i++)
                remove(i);
        }

        void insert(const T* obj) {
                        // if (findNode(m_start, (uint64_t)&obj) != nullptr) {
                        // 	return; // object already exists
                        // }
            Node* node = newNode((uint64_t)obj);
            if (m_start == nullptr) {
                m_start = node;
                m_end = node;
            } else {
                m_end->next = node;
                node->previous = m_end;
                m_end = node;
            }
            m_count++;
        }

        T* get(uint64_t index) const {
            if (index >= m_count)
                return nullptr;
            Node* temp = m_start;
            for (uint64_t i = 0; i < index; i++) {
                if (temp == nullptr)
                    return nullptr;
                temp = temp->next;
            }
            if (temp == nullptr)
                return nullptr;
            return (T*)(temp->data);
        }

        uint64_t getIndex(const T* obj) const {
            Node* temp = m_start;
            for (uint64_t i = 0; i < m_count; i++) {
                if (temp == nullptr)
                    return UINT64_MAX;
                if (temp->data == (uint64_t)obj)
                    return i;
                temp = temp->next;
            }
            return UINT64_MAX;
        }

        void remove(uint64_t index) {
            deleteNode(m_start, (uint64_t)get(index));
            m_count--;
        }

        void remove(const T* obj) {
            deleteNode(m_start, (uint64_t)obj);
            m_count--;
        }

        void Enumerate(std::function<void(T*)> func) const {
            Node* temp = m_start;
            for (uint64_t i = 0; i < m_count; i++) {
                                // if (temp == nullptr)
                                // 	return;
                func((T*)(temp->data));
                temp = temp->next;
            }
        }

        void Enumerate(std::function<bool(T*, uint64_t index)> func, uint64_t starting_index = 0) const {
            Node* temp = m_start;
            for (uint64_t i = 0; i < starting_index; i++) {
                                // if (temp == nullptr)
                                // 	return;
                temp = temp->next;
            }
            for (uint64_t i = starting_index; i < m_count; i++) {
                                // if (temp == nullptr)
                                // 	return;
                if (!func((T*)(temp->data), i))
                    return;
                temp = temp->next;
            }
        }

                // Enumerate in reserve order
        void EnumerateReverse(std::function<bool(T*)> func) const {
            Node* temp = m_end;
            for (uint64_t i = 0; i < m_count; i++) {
                                // if (temp == nullptr)
                                // 	return;
                if (!func((T*)(temp->data)))
                    return;
                temp = temp->previous;
            }
        }

        void clear() {
            for (uint64_t i = 0; i < m_count; i++)
                deleteNode(m_start, m_start);
            m_count = 0;
            m_end = nullptr;
        }

        uint64_t getCount() const {
            return m_count;
        }

       private:
        uint64_t m_count;
        Node* m_start;
        Node* m_end;
    };
} // namespace LinkedList

extern bool operator==(LinkedList::Node left, LinkedList::Node right);

#endif /* _LINKED_LIST_HPP */
