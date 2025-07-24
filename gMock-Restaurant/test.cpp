#include "gmock/gmock.h"
#include "booking_scheduler.cpp"

using namespace testing;

class BookingFixture : public Test {
public:
	tm getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0, min,hour, day, mon - 1, year - 1900, 0, 0, -1 };
		mktime(&result);
		return result;
	}
};

TEST_F(BookingFixture, 예약은정시에만가능하다정시가아닌경우예약불가) {
	// Arrange
	tm notOntheHour = getTime(2021, 3, 26, 9, 5);

	Customer customer{ "Fake name", "010-1234-5678" };
	Schedule* schedule = new Schedule{ notOntheHour, 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	// Act
	EXPECT_THROW({
		bookingScheduler.addSchedule(schedule);
		}, std::runtime_error);

	// Assert
	// expected runtime exception
}

TEST_F(BookingFixture, 예약은정시에만가능하다정시인경우예약가능) {
	// Arrange
	tm OntheHour = getTime(2021, 3, 26, 9, 0);

	Customer customer{ "Fake name", "010-1234-5678" };
	Schedule* schedule = new Schedule{ OntheHour, 1, customer };
	BookingScheduler bookingScheduler{ 3 };

	// Act
	bookingScheduler.addSchedule(schedule);

	// Assert
	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST(BookingSchedulerTest, 시간대별인원제한이있다같은시간대에Capacity초과할경우예외발생) {

}

TEST(BookingSchedulerTest, 시간대별인원제한이있다시간대가다르면Capacity차있어도스케쥴추가성공) {

}

TEST(BookingSchedulerTest, 예약완료시SMS는무조건발송) {

}

TEST(BookingSchedulerTest, 이메일이없는경우에는이메일미발송) {

}

TEST(BookingSchedulerTest, 이메일이있는경우에는이메일발송) {

}

TEST(BookingSchedulerTest, 현재날짜가일요일인경우예약불가함예외처리발생) {

}

TEST(BookingSchedulerTest, 현재날짜가일요일이아닌경우예약가능) {

}

int main() {
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}