#ifndef I_ORDER_STATISTIC_H
#define I_ORDER_STATISTIC_H

class IOrderStatistic {
public:
    virtual int add(double value) = 0;
    virtual bool retract(int id) = 0;
    virtual double getMedian() = 0;
    virtual int getSize() const = 0;

    virtual ~IOrderStatistic() = default;
};

#endif
