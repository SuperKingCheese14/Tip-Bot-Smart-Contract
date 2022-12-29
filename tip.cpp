#include "tip.hpp"

void tip::deposit (name from, name to, asset quantity, const std::string& memo) {
  auto self = get_self();

  if (to == self) {

    add_deposit(get_first_receiver(), stoull(memo), quantity, self);
  
    print("Deposit complete to:", name{to});
  }
}

void tip::withdraw (name contract, uint64_t from, name to, asset quantity, const std::string& memo) {
  auto self = get_self();
  require_auth(self);
  
  sub_balance(contract, from, quantity);

  // Inline transfer
  tipped::transfer_action t_action( contract, {self, "active"_n} );
  t_action.send(self, to, quantity, memo);
   
  print("Withdrawn from: ", from, " To: ", name{to});
}

void tip::tip (name contract,
                  uint64_t from,
                  uint64_t to,
                  asset quantity,
                  const std::string memo) {
  require_auth(get_self());
  
  sub_balance(contract, from, quantity);
  add_balance(contract, to, quantity, get_self());
  
  print("Transferred from: ", name{from}, " To: ", name{to});
}

void tip::add_balance( name contract, uint64_t owner, asset value, name ram_payer ) {
   accounts to_acnts( get_self(), owner );
   auto to = to_acnts.find( contract.value );
   
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.owner = owner;
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void tip::add_deposit( name account, uint64_t memo, asset quantity, name ram_payer ) {
   deposits to_deps( get_self(), account );
   auto to = to_deps.find( account.value );
   
   if( to == to_deps.end() ) {
     to_deps.emplace( ram_payer, [&]( auto& a ) {
       a.owner = account;
       a.quantity = quantity;
       a.memo = owner;
     });
   } else {
      to_deps.modify( to, same_payer, [&]( auto& a ) {
        a.quantity += quantity;
      });
   }
}

void tip::sub_balance( name contract, uint64_t owner, asset value ) {
   accounts from_acnts( get_self(), owner );
   
   const auto& from = from_acnts.get( contract.value, "your discord account does not have a balance, deposit or receive a tip first." );
   check( from.balance.amount >= value.amount, std::string("not enough balance in your discord account. Current balance: ") + from.balance.to_string().c_str() );

   if (from.balance.amount == value.amount) {
     from_acnts.erase(from);
   } else {
     from_acnts.modify( from, same_payer, [&]( auto& a ) {
       a.balance -= value;
     });
   }
}

// Asserts if the supplied account doesn't have an entry for a given token
void tip::verify_entry(name account, name currency_contact, asset currency) {
    accounts accountstable(currency_contact, account.value);
    auto ac = accountstable.find(currency.symbol.code().raw());
    check(ac != accountstable.end(), "must have entry for token (claim token first)");
}
