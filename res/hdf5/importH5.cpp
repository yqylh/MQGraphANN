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

using namespace HighFive;

const std::string FILE_NAME("fashion-mnist-784-euclidean.hdf5");
const std::string DATASET_NAME("train");
const size_t size_dataset = 20;

// read our data back
void read_dataset() {
    // we open the existing hdf5 file we created before
    File file(FILE_NAME, File::ReadOnly);

    std::vector<std::vector<float> > read_data;

    // we get the dataset
    DataSet dataset = file.getDataSet(DATASET_NAME);

    // we convert the hdf5 dataset to a single dimension vector
    dataset.read(read_data);

    for (size_t i = 0; i < read_data.size(); ++i)
    {
        std::cout << read_data[i].size() << " ";
    }
    std::cout << read_data.size() << " ";
    std::cout << "\n";
}

int main(void) {
    read_dataset();
    return 0;
}