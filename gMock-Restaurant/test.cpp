#include "gmock/gmock.h"
#include "booking_scheduler.cpp"
#include "testable_sms_sender.cpp";
#include "testable_mail_sender.cpp";
#include "sunday_booking_scheduler.cpp"
#include "monday_booking_scheduler.cpp"
#include "testable_booking_scheduler.cpp"

using namespace testing;

class BookingFixture : public Test {
protected:
	void SetUp() override {
		this->NOT_ON_THE_HOUR = getTime(2021, 3, 26, 9, 5);
		this->ON_THE_HOUR = getTime(2021, 3, 26, 9, 0);

		bookingScheduler.setSmsSender(&testableSmsSender);
		bookingScheduler.setMailSender(&testableMailSender);
	}
public:
	tm getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0, min,hour, day, mon - 1, year - 1900, 0, 0, -1 };
		mktime(&result);
		return result;
	}
	tm plusHour(tm base, int hour) {
		base.tm_hour += hour;
		mktime(&base);
		return base;
	}

	tm NOT_ON_THE_HOUR;
	tm ON_THE_HOUR;
	Customer CUSTOMER{ "Fake name", "010-1234-5678" };
	Customer customerWithMail{ "Fake Name", "010-1234-5678", "test@test.com" };


	const int UNDER_CAPACITY = 1;
	const int CAPACITY_PER_HOUR = 3;

	BookingScheduler bookingScheduler{ CAPACITY_PER_HOUR };
	TestableSmsSender testableSmsSender;
	TestableMailSender testableMailSender;

};

TEST_F(BookingFixture, 예약은정시에만가능하다정시가아닌경우예약불가) {
	// Arrange
	Schedule* schedule = new Schedule{ NOT_ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };

	// Act
	EXPECT_THROW({
		bookingScheduler.addSchedule(schedule);
		}, std::runtime_error);

	// Assert
	// expected runtime exception
}

TEST_F(BookingFixture, 예약은정시에만가능하다정시인경우예약가능) {
	// Arrange
	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };

	// Act
	bookingScheduler.addSchedule(schedule);

	// Assert
	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingFixture, 시간대별인원제한이있다같은시간대에Capacity초과할경우예외발생) {
	// Arrange
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	// act
	try {
		Schedule* newSchedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };
		bookingScheduler.addSchedule(newSchedule);
		FAIL(); // Exception 발생함으로 FAIL함수에 도달하지 않는다
	}
	catch (std::runtime_error& e) {
		// assert
		EXPECT_EQ(string{ e.what() }, string{ "Number of people is over restaurant capacity per hour" });
	}
}

TEST_F(BookingFixture, 시간대별인원제한이있다시간대가다르면Capacity차있어도스케쥴추가성공) {
	// Arrange
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	// Act
	tm differentHour = plusHour(ON_THE_HOUR, 1);
	Schedule* newSchedule = new Schedule{ differentHour, UNDER_CAPACITY, CUSTOMER };
	bookingScheduler.addSchedule(newSchedule);

	// Assert
	EXPECT_EQ(true, bookingScheduler.hasSchedule(newSchedule));
}

TEST_F(BookingFixture, 예약완료시SMS는무조건발송) {
	// Arrange
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };

	// Act
	bookingScheduler.addSchedule(schedule);

	// Assert
	EXPECT_EQ(true, testableSmsSender.isSendMethodIsCalled());
}

TEST_F(BookingFixture, 이메일이없는경우에는이메일미발송) {
	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };
	bookingScheduler.setMailSender(&testableMailSender);

	// Act
	bookingScheduler.addSchedule(schedule);

	// Assert
	EXPECT_EQ(0, testableMailSender.getCountSendMailMethodIsCalled());
}

TEST_F(BookingFixture, 이메일이있는경우에는이메일발송) {
	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, customerWithMail };

	// Act
	bookingScheduler.addSchedule(schedule);

	// Assert
	EXPECT_EQ(1, testableMailSender.getCountSendMailMethodIsCalled());
}

TEST_F(BookingFixture, 현재날짜가일요일인경우예약불가함예외처리발생) {
	//BookingScheduler* bookingScheduler = new SundayBookingScheduler{ CAPACITY_PER_HOUR };
	BookingScheduler* bookingScheduler = new TestableBookingScheduler{ CAPACITY_PER_HOUR, getTime(2025, 7, 27, 17, 0)};

	try {
		Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY,customerWithMail };
		bookingScheduler->addSchedule(schedule);
		FAIL();
	}
	catch (std::runtime_error& e) {
		EXPECT_EQ(string{ e.what() }, string{ "Booking system is not available on sunday" });
	}
}

TEST_F(BookingFixture, 현재날짜가일요일이아닌경우예약가능) {
	//BookingScheduler* bookingScheduler = new MondayBookingScheduler{ CAPACITY_PER_HOUR };
	BookingScheduler* bookingScheduler = new TestableBookingScheduler{ CAPACITY_PER_HOUR, getTime(2025, 7, 28, 17, 0) };

	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY,customerWithMail };
	bookingScheduler->addSchedule(schedule);

	EXPECT_EQ(true, bookingScheduler->hasSchedule(schedule));
}

int main() {
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}