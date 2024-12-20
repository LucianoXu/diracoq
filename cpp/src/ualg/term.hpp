#pragma once

#include <string>
#include <set>
#include <boost/container_hash/hash_fwd.hpp>
#include <boost/functional/hash.hpp>

namespace ualg {

    inline std::string data_to_string(const std::string& str) {
        return str;
    }

    inline std::size_t hash_value(const std::string& str) {
        boost::hash<std::string> string_hash;
        return string_hash(str);
    }

    inline std::string data_to_string(const int& i) {
        return std::to_string(i);
    }

    inline std::size_t hash_value(const int& i) {
        return i;
    }


    /**
     * @brief The abstract class for terms.
     * 
     * @tparam T The type of the head(data) of the term. The function std::string data_to_string(const T&) and std::size_t hash_value(const T&) should be defined.
     */
    template <class T>
    class Term {
    protected:
        T head;
        std::size_t hvalue;

    public: 
        std::size_t hash_value() const;

        const T& get_head() const;

        virtual bool operator == (const Term& other) const = 0;

        bool operator != (const Term& other) const;

        virtual bool operator < (const Term& other) const = 0;

        std::size_t get_term_size() const;

        bool is_atomic() const;

        virtual std::string to_string(std::shared_ptr<const std::map<T, std::string>> head_naming=nullptr) const = 0;

        virtual ~Term() = default;
    
    };

    template <class T>
    inline std::size_t hash_value(const Term<T>& term) {
        return term.hash_value();
    }

    /**
     * @brief Get all the unique nodes in the term.
     * 
     * @tparam T The type of the head(data) of the term.
     * @param term 
     * @return std::set<const Term*> 
     */
    template <class T>
    std::set<const Term<T>*> get_all_nodes(const Term<T>* term);

    //////////////////////////////////////////////////////////////////////////
    // Normal Terms

    template <class T>
    using ListArgs = std::vector<const Term<T>*>;

    template <class T>
    inline std::size_t calc_hash_normal(const T& head, const ListArgs<T>& args) {
        std::size_t seed = 0;
        boost::hash_combine(seed, hash_value(head));
        for (const auto& arg : args) {
            boost::hash_combine(seed, arg);
        }
        return seed;
    }

    using NormalTermPos = std::vector<unsigned int>;

    template <class T>
    class NormalTerm : public Term<T> {
    private:
        ListArgs<T> args;

    public:
        // The data type for the position of the argument in the term. Positions are lists of indices indicating the order of the subterm. Indices start from 0.

        NormalTerm(const T& head);
        NormalTerm(const T& head, const ListArgs<T>& normal_args);
        NormalTerm(const T& head, ListArgs<T>&& normal_args);

        const ListArgs<T>& get_args() const;

        bool operator == (const Term<T>& other) const;
        bool operator < (const Term<T>& other) const;

        std::string to_string(std::shared_ptr<const std::map<T, std::string>> head_naming=nullptr) const;

        const NormalTerm<T>* get_subterm(const NormalTermPos& pos) const;
    };

    template <class T>
    inline std::size_t hash_value(const NormalTerm<T>& term) {
        return term.hash_value();
    }

    //////////////////////////////////////////////////////////////////////////
    // TermCountMapping and corresponding functions

    template <class T>
    using TermCountMapping = std::map<const Term<T>*, unsigned int>;

    template <class T>
    inline void add_TermCountMapping(TermCountMapping<T>& mapping, const Term<T>* term, unsigned int count) {
        if (mapping.find(term) != mapping.end()) {
            mapping[term] += count;
        }
        else {
            mapping[term] = count;
        }
    }

    template <class T>
    inline void subtract_TermCountMapping(TermCountMapping<T>& mapping, const Term<T>* term, unsigned int count) {
        if (mapping.find(term) != mapping.end()) {
            mapping[term] -= count;
            if (mapping[term] == 0) {
                mapping.erase(term);
            }
        }
        else {
            throw std::runtime_error("Term not found in the mapping.");
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // C Terms

    template <class T>
    inline std::size_t calc_hash_c(const T& head, const TermCountMapping<T>& args) {
        std::size_t seed = 0;
        boost::hash_combine(seed, hash_value(head));
        for (const auto& arg : args) {
            boost::hash_combine(seed, arg.first);
            boost::hash_combine(seed, arg.second);
        }
        return seed;
    }
    
    template <class T>
    class CTerm : public Term<T> {
    private:
        TermCountMapping<T> args;

    public:
        CTerm(const T& head);
        CTerm(const T& head, const TermCountMapping<T>& c_args);
        

        const TermCountMapping<T>& get_args() const;

        bool operator == (const Term<T>& other) const;
        bool operator < (const Term<T>& other) const;

        std::string to_string(std::shared_ptr<const std::map<T, std::string>> head_naming=nullptr) const;

    };

    template <class T>
    inline std::size_t hash_value(const CTerm<T>& term) {
        return term.hash_value();
    }

    //////////////////////////////////////////////////////////////////////////
    // AC Terms

    template <class T>
    inline std::size_t calc_hash_ac(const T& head, const TermCountMapping<T>& args) {
        std::size_t seed = 0;
        boost::hash_combine(seed, hash_value(head));
        for (const auto& arg : args) {
            boost::hash_combine(seed, arg.first);
            boost::hash_combine(seed, arg.second);
        }
        return seed;
    }

    template <class T>
    class ACTerm : public Term<T> {
    private:
        TermCountMapping<T> args;

    public:
        ACTerm(const T& head);
        ACTerm(const T& head, const TermCountMapping<T>& ac_args);
        

        const TermCountMapping<T>& get_args() const;

        bool operator == (const Term<T>& other) const;
        bool operator < (const Term<T>& other) const;

        std::string to_string(std::shared_ptr<const std::map<T, std::string>> head_naming=nullptr) const;

    };

    template <class T>
    inline std::size_t hash_value(const ACTerm<T>& term) {
        return term.hash_value();
    }



    /////////////////////////////////////////////////////////////////
    // Implementations


    ///////////////
    // Term

    template <class T>
    std::size_t Term<T>::hash_value() const {
        return this->hvalue;
    }

    template <class T>
    const T& Term<T>::get_head() const {
        return this->head;
    }

    template <class T>
    bool Term<T>::operator != (const Term<T>& other) const {
        return !(*this == other);
    }

    template <class T>
    std::size_t Term<T>::get_term_size() const {
        return get_all_nodes(this).size();
    }

    template <class T>
    bool Term<T>::is_atomic() const {
        return get_term_size() == 1;
    }

    // get_all_nodes

    template <class T>
    void _get_all_nodes(const Term<T>* term, std::set<const Term<T>*>& nodes) {
        if (nodes.find(term) != nodes.end()) {
            return;
        }
        nodes.insert(term);
        if (typeid(*term) == typeid(NormalTerm<T>)) {
            const NormalTerm<T>* normal_term = static_cast<const NormalTerm<T>*>(term);
            for (const auto& arg : normal_term->get_args()) {
                _get_all_nodes(arg, nodes);
            }
        }
        else if (typeid(*term) == typeid(ACTerm<T>)) {
            const ACTerm<T>* ac_term = static_cast<const ACTerm<T>*>(term);
            for (const auto& arg : ac_term->get_args()) {
                _get_all_nodes(arg.first, nodes);
            }
        }
        else {
            throw std::runtime_error("Unknown term type.");
        }
    }

    template <class T>
    std::set<const Term<T>*> get_all_nodes(const Term<T>* term) {
        std::set<const Term<T>*> nodes;
        _get_all_nodes(term, nodes);
        return nodes;
    }

    ////////////////
    // Normal Term

    template <class T>
    NormalTerm<T>::NormalTerm(const T& head) {
        this->head = head;
        this->hvalue = calc_hash_normal(head, this->args);
    }

    template <class T>
    NormalTerm<T>::NormalTerm(const T& head, const ListArgs<T>& normal_args) {
        this->head = head;
        this->args = normal_args;
        this->hvalue = calc_hash_normal(head, this->args);
    }

    template <class T>
    NormalTerm<T>::NormalTerm(const T& head, ListArgs<T>&& normal_args) {
        this->head = head;
        this->args = std::move(normal_args);
        this->hvalue = calc_hash_normal(head, this->args);
    }

    template <class T>
    const ListArgs<T>& NormalTerm<T>::get_args() const {
        return this->args;
    }

    template <class T>
    bool NormalTerm<T>::operator == (const Term<T>& other) const {
        if (this == &other) {
            return true;
        }
        if (typeid(other) != typeid(NormalTerm<T>)) {
            return false;
        }
        const NormalTerm<T>& other_term = static_cast<const NormalTerm<T>&>(other);
        if (this->head != other_term.head) {
            return false;
        }
        if (this->args != other_term.args) {
            return false;
        }
        return true;
    }

    template <class T>
    bool NormalTerm<T>::operator < (const Term<T>& other) const {
        if (this->hvalue < other.hash_value()) {
            return true;
        }

        if (typeid(other) != typeid(NormalTerm<T>)) {
            return typeid(*this).before(typeid(other));
        }

        const NormalTerm<T>& other_term = static_cast<const NormalTerm<T>&>(other);

        if (this->head != other_term.head) {
            return this->head < other_term.head;
        }
        return this->args < other_term.args;
    }

    template <class T>
    std::string NormalTerm<T>::to_string(std::shared_ptr<const std::map<T, std::string>> head_naming) const {
        std::string str;
        if (head_naming != nullptr) {
            str = head_naming->at(this->head);
        }
        else {
            str = data_to_string(this->head);
        }
        if (args.size() > 0) {
            str += "(";
            for (const auto& arg : this->args) {
                str += arg->to_string(head_naming) + " ";
            }
            str.pop_back();
            str += ")";
        }
        return str;
    }

    template <class T>
    const NormalTerm<T>* NormalTerm<T>::get_subterm(const NormalTermPos& pos) const {
        if (pos.size() == 0) {
            return this;
        }

        if (pos[0] >= this->args.size()) {
            throw std::runtime_error("Position out of range.");
        }

        return static_cast<const NormalTerm<T>*>(this->args[pos[0]]);
    }


    ////////////////////////////////////////////////////
    // C Terms

    template <class T>
    CTerm<T>::CTerm(const T& head) {
        this->head = head;
        this->hvalue = calc_hash_c(head, this->args);
    }

    template <class T>
    CTerm<T>::CTerm(const T& head, const TermCountMapping<T>& c_args) {
        this->head = head;
        this->args = std::move(c_args);
        this->hvalue = calc_hash_c(head, this->args);
    }

    template <class T>
    const TermCountMapping<T>& CTerm<T>::get_args() const {
        return this->args;
    }

    template <class T>
    bool CTerm<T>::operator == (const Term<T>& other) const {
        if (this == &other) {
            return true;
        }
        if (typeid(other) != typeid(CTerm<T>)) {
            return false;
        }
        const CTerm<T>& other_term = static_cast<const CTerm<T>&>(other);
        if (this->head != other_term.head) {
            return false;
        }
        if (this->args != other_term.args) {
            return false;
        }
        return true;
    }

    template <class T>
    bool CTerm<T>::operator < (const Term<T>& other) const {
        if (this->hvalue < other.hash_value()) {
            return true;
        }

        if (typeid(other) != typeid(CTerm<T>)) {
            return typeid(*this).before(typeid(other));
        }

        const CTerm<T>& other_term = static_cast<const CTerm<T>&>(other);

        if (this->head != other_term.head) {
            return this->head < other_term.head;
        }
        return this->args < other_term.args;
    }

    template <class T>
    std::string CTerm<T>::to_string(std::shared_ptr<const std::map<T, std::string>> head_naming) const {
        std::string str;
        if (head_naming != nullptr) {
            str = head_naming->at(this->head);
        }
        else {
            str = data_to_string(this->head);
        }

        if (this->args.size() > 0) {
            str += "(";
            for (const auto& arg : this->args) {
                str += arg.first->to_string(head_naming) + ":" + std::to_string(arg.second) + " ";
            }
            str.pop_back();
            str += ")";
        }
        return str;
    }

    ////////////////
    // AC Terms

    template <class T>
    ACTerm<T>::ACTerm(const T& head) {
        this->head = head;
        this->hvalue = calc_hash_ac(head, this->args);
    }

    template <class T>
    ACTerm<T>::ACTerm(const T& head, const TermCountMapping<T>& ac_args) {
        this->head = head;
        this->args = TermCountMapping<T>{};

        // Check and flatten the arguments
        for (const auto& arg : ac_args) {
            // if should be flattened
            if (typeid(*arg.first) == typeid(ACTerm) && arg.first->get_head() == head) {
                // cast the term to AC term
                auto sub_term = static_cast<const ACTerm*>(arg.first);
                for (const auto& sub_arg : sub_term->get_args()) {
                    add_TermCountMapping(args, sub_arg.first, arg.second * sub_arg.second);
                }
            }
            else {
                add_TermCountMapping(this->args, arg.first, arg.second);
            }
        }
        this->hvalue = calc_hash_ac(head, this->args);
    }

    template <class T>
    const TermCountMapping<T>& ACTerm<T>::get_args() const {
        return this->args;
    }

    template <class T>
    bool ACTerm<T>::operator == (const Term<T>& other) const {
        if (this == &other) {
            return true;
        }
        if (typeid(other) != typeid(ACTerm<T>)) {
            return false;
        }
        const ACTerm<T>& other_term = static_cast<const ACTerm<T>&>(other);
        if (this->head != other_term.head) {
            return false;
        }
        if (this->args != other_term.args) {
            return false;
        }
        return true;
    }

    template <class T>
    bool ACTerm<T>::operator < (const Term<T>& other) const {
        if (this->hvalue < other.hash_value()) {
            return true;
        }

        if (typeid(other) != typeid(ACTerm<T>)) {
            return typeid(*this).before(typeid(other));
        }

        const ACTerm<T>& other_term = static_cast<const ACTerm<T>&>(other);

        if (this->head != other_term.head) {
            return this->head < other_term.head;
        }
        return this->args < other_term.args;
    }

    template <class T>
    std::string ACTerm<T>::to_string(std::shared_ptr<const std::map<T, std::string>> head_naming) const {
        std::string str;
        if (head_naming != nullptr) {
            str = head_naming->at(this->head);
        }
        else {
            str = data_to_string(this->head);
        }

        if (args.size() > 0) {
            str += "(";
            for (const auto& arg : this->args) {
                str += arg.first->to_string(head_naming) + ":" + std::to_string(arg.second) + " ";
            }
            str.pop_back();
            str += ")";
        }
        return str;
    }

}   // namespace ualg