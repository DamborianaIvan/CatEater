const toFeederResponse = (feeder) => ({
  feederId: feeder.feederId,
  feederName: feeder.feederName,
  feederLogo: feeder.feederLogo,
  feederQuantity: feeder.feederQuantity,
  status: feeder.getStatus(),
  motorState: feeder.motorInfo?.motorState ?? false,
  portions: feeder.motorInfo?.portions ?? 1,
  lastConnection: feeder.lastConection
});

module.exports = {
  toFeederResponse
};
