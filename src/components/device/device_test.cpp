#include<gtest/gtest.h>
#include "device.hpp"
#include<boost/uuid/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include <iostream>
#include<boost/uuid/uuid_io.hpp>

class DeviceTest : public ::testing::Test {
protected:
	boost::uuids::random_generator gen;
	/* Device d1; */

	void SetUp() override {
		/* d1 = gen(); */
	}

	/* void TearDown() override { } */

};

TEST_F(DeviceTest, Structure) {
}
