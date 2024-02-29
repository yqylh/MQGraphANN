/*
 *  Copyright (c), 2017, Adrien Devresse
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

template <typename T>
std::vector<std::vector<T>> read_dataset(std::string FILE_NAME, std::string DATASET_NAME) {
    // we open the existing hdf5 file we created before
    HighFive::File file(FILE_NAME, HighFive::File::ReadOnly);
    std::vector<std::vector<T> > read_data;
    // we get the dataset
    HighFive::DataSet dataset = file.getDataSet(DATASET_NAME);
    // we convert the hdf5 dataset to a single dimension vector
    dataset.read(read_data);
    return read_data;
}