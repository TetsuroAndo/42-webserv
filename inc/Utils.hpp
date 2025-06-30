/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 16:35:40 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 17:11:00 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <algorithm>
#include <cctype>

struct NotSpace {
    bool operator()(char c) const {
        return !std::isspace(static_cast<unsigned char>(c));
    }
};

// 前方から空白をトリムする関数
std::string &ft_ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), NotSpace()));
    return s;
}

// 後方から空白をトリムする関数
std::string &ft_rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), NotSpace()).base(), s.end());
    return s;
}

// 両端から空白をトリムする関数（新しい文字列を返すバージョン）
std::string ft_trim(const std::string &s) {
    std::string copy = s;
    return ft_ltrim(ft_rtrim(copy));
}