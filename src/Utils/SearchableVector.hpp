#pragma once

#include <unordered_map>
#include <vector>

template <typename KEY_T, typename VALUE_T>
class SearchableVector {
private:
    std::unordered_map<KEY_T, std::pair<size_t, VALUE_T>> m_Dico;
    std::vector<VALUE_T> m_Array;

public:
    void clear();
    bool empty() const;
    size_t size() const;
    VALUE_T& operator[](const size_t& vIdx);
    VALUE_T& operator[](const KEY_T& vKey);
    VALUE_T at(const size_t& vIdx) const;
    VALUE_T at(const KEY_T& vKey) const;
    typename std::vector<VALUE_T>::iterator begin();
    typename std::vector<VALUE_T>::iterator end();
    typename std::vector<VALUE_T>::reverse_iterator rbegin();
    typename std::vector<VALUE_T>::reverse_iterator rend();
    typename std::vector<VALUE_T>::const_iterator begin() const;
    typename std::vector<VALUE_T>::const_iterator end() const;
    typename std::vector<VALUE_T>::const_reverse_iterator rbegin() const;
    typename std::vector<VALUE_T>::const_reverse_iterator rend() const;
    /// <summary>
    /// will add the value for the key
    /// </summary>
    bool try_add(const KEY_T& vKey, VALUE_T vValue);
    /// <summary>
    /// will set the value of the key if the key exist only
    /// </summary>
    bool try_set_if_existing(const KEY_T& vKey, VALUE_T vValue);
    /// <summary>
    /// return true is the key already exist
    /// </summary>
    bool exist(const KEY_T& vKey) const;
    bool getValue(const KEY_T& vKey, VALUE_T& vOutValue) const;
};

template <typename KEY_T, typename VALUE_T>
VALUE_T& SearchableVector<KEY_T, VALUE_T>::operator[](const size_t& vIdx) {
    return m_Array[vIdx];
}

template <typename KEY_T, typename VALUE_T>
VALUE_T& SearchableVector<KEY_T, VALUE_T>::operator[](const KEY_T& vKey) {
    return m_Dico[vKey].second;
}

template <typename KEY_T, typename VALUE_T>
VALUE_T SearchableVector<KEY_T, VALUE_T>::at(const size_t& vIdx) const {
    return m_Array.at(vIdx);
}

template <typename KEY_T, typename VALUE_T>
VALUE_T SearchableVector<KEY_T, VALUE_T>::at(const KEY_T& vKey) const {
    return m_Dico.at(vKey).second;
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::iterator SearchableVector<KEY_T, VALUE_T>::begin() {
    return m_Array.begin();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::iterator SearchableVector<KEY_T, VALUE_T>::end() {
    return m_Array.end();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::reverse_iterator SearchableVector<KEY_T, VALUE_T>::rbegin() {
    return m_Array.rbegin();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::reverse_iterator SearchableVector<KEY_T, VALUE_T>::rend() {
    return m_Array.rend();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::const_iterator SearchableVector<KEY_T, VALUE_T>::begin() const {
    return m_Array.cbegin();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::const_iterator SearchableVector<KEY_T, VALUE_T>::end() const {
    return m_Array.cend();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::const_reverse_iterator SearchableVector<KEY_T, VALUE_T>::rbegin() const {
    return m_Array.crbegin();
}

template <typename KEY_T, typename VALUE_T>
typename std::vector<VALUE_T>::const_reverse_iterator SearchableVector<KEY_T, VALUE_T>::rend() const {
    return m_Array.crend();
}

template <typename KEY_T, typename VALUE_T>
size_t SearchableVector<KEY_T, VALUE_T>::size() const {
    return m_Array.size();
}

template <typename KEY_T, typename VALUE_T>
void SearchableVector<KEY_T, VALUE_T>::clear() {
    m_Dico.clear();
    m_Array.clear();
}

template <typename KEY_T, typename VALUE_T>
bool SearchableVector<KEY_T, VALUE_T>::empty() const {
    return m_Array.empty();
}

template <typename KEY_T, typename VALUE_T>
bool SearchableVector<KEY_T, VALUE_T>::try_add(const KEY_T& vKey, VALUE_T vValue) {
    if (!exist(vKey)) {
        m_Dico[vKey] = std::make_pair(m_Array.size(), vValue);
        m_Array.push_back(vValue);
        return true;
    }

    return false;
}

template <typename KEY_T, typename VALUE_T>
bool SearchableVector<KEY_T, VALUE_T>::try_set_if_existing(const KEY_T& vKey, VALUE_T vValue) {
    if (exist(vKey)) {
        auto key = m_Dico.at(vKey);
        m_Dico[vKey] = std::make_pair(key.first, vValue);
        m_Array[key.first] = vValue;
        return true;
    }

    return false;
}

template <typename KEY_T, typename VALUE_T>
bool SearchableVector<KEY_T, VALUE_T>::exist(const KEY_T& vKey) const {
    return (m_Dico.find(vKey) != m_Dico.end());
}

template <typename KEY_T, typename VALUE_T>
bool SearchableVector<KEY_T, VALUE_T>::getValue(const KEY_T& vKey, VALUE_T& vOutValue) const {
    if (exist(vKey)) {
        vOutValue = m_Dico.at(vKey).second;
        return true;
    }
    vOutValue = {};
    return false;
}
