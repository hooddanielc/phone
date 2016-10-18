import Ember from 'ember';

export default Ember.Component.extend({
  store: Ember.inject.service('store'),
  classNames: ['contacts'],
  input: '',
  speedDialActive: true,
  recentsActive: false,
  contactsActive: false,

  oneTabSelected: function () {
    const properties = ['speedDialActive', 'recentsActive', 'contactsActive'];
    let selectedTabs = 0;

    properties.forEach((property) => {
      if (this.get(property)) {
        ++selectedTabs;
      }
    });

    return selectedTabs === 1;
  }.property('activeTab'),

  contacts: function () {
    return this.get('store').findAll('contact');
  }.property('activeTab'),

  recents: function () {
    return this.get('store').findAll('contact');
  }.property('activeTab'),

  speedDialContacts: function () {
    return this.get('store').findAll('contact');
  }.property('activeTab'),

  isLoading: function () {
    if (this.get('speedDialActive')) {
      return !this.get('speedDialContacts.isFulfilled');
    } else if (this.get('contactsActive')) {
      return !this.get('contacts.isFulfilled');
    } else if (this.get('recentsActive')) {
      return !this.get('recents.isFulfilled');
    }
  }.property(
    'speedDialContacts.isFulfilled',
    'contacts.isFulfilled',
    'recents.isFulfilled'
  ),

  activeTab: function () {
    if (this.get('speedDialActive')) {
      return 'speedDialActive';
    } else if (this.get('contactsActive')) {
      return 'contactsActive';
    } else if (this.get('recentsActive')) {
      return 'recentsActive';
    }
  }.property('speedDialActive', 'recentsActive', 'contactsActive'),

  actions: {
    switchTab: function (tab) {
      this.setProperties({
        speedDialActive: false,
        recentsActive: false,
        contactsActive: false
      });

      if (typeof this.get(tab + 'Active') !== 'undefined') {
        this.set(tab + 'Active', true);
      }
    }
  }
});
