import Ember from 'ember';
import FixtureAdapter from 'ember-data-fixture-adapter';

export default FixtureAdapter.extend({
  // simulate http request
  findAll: function () {
    return this._super.apply(this, arguments).then((result) => {
      return new Ember.RSVP.Promise((resolve) => {
        setTimeout(() => {
          resolve(result);
        }, 2000);
      })
    });
  }
});
