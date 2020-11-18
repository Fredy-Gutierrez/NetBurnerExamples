
class MyModAlloc : public DHCP::BlockAllocator
{
    public:
    MyModAlloc();
    ~MyModAlloc();
    virtual bool OfferLease( DHCP::DhcpLeaseRequest *pLease, int intfNum );
    virtual bool RequestLease( DHCP::DhcpLeaseRequest *pLease, int intfNum );
    virtual bool SetStaticLease( DHCP::DhcpLeaseRequest *pLease );
};





