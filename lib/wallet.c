#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wallet.h"

/**
 * Initializes an empty wallet.
 */
void wallet_init(wallet_t *wallet) {
  // Implement
  wallet->head = NULL;
  pthread_mutex_init(&wallet->lock, NULL);
  pthread_cond_init(&wallet->cond, NULL);
}


/**
 * Returns the amount of a given `resource` in the given `wallet`.
 */
int wallet_get(wallet_t *wallet, const char *resource) {
  // Implement this
  for (wallet_resource *curr=wallet->head; curr!=NULL; curr=curr->next) {
    if (!strcmp(curr->resource_name, resource)) {
      return curr->amount;
    }
  }
  return 0;
}

/**
 * Modifies the amount of a given `resource` in a given `wallet by `delta`.
 * - If `delta` is negative, this function MUST NOT RETURN until the resource can be satisfied.
 *   (Ths function MUST WAIT until the wallet has enough resources to satisfy the request;
 *    there are several ways to accomplish this waiting and it does not have to be fancy.)
 */
void wallet_change_resource(wallet_t *wallet, const char *resource, const int delta) {
  // Implement this
  pthread_mutex_lock(&wallet->lock);
  if (delta < 0) {
    for (wallet_resource *curr=wallet->head; curr!=NULL; curr=curr->next) {
      if (!strcmp(curr->resource_name, resource)) {
        while (curr->amount < abs(delta)) {
          pthread_cond_wait(&wallet->cond, &wallet->lock);
        }
        curr -> amount = curr->amount + delta;
        pthread_mutex_unlock(&wallet->lock);
        return;
      }
    }
    wallet_resource* to_add = malloc(255);
    to_add -> next = wallet -> head;
    wallet -> head = to_add;
    wallet -> head -> resource_name = resource;
    wallet -> head -> amount = 0;
    while (to_add -> amount < abs(delta)) {
      pthread_cond_wait(&wallet->cond, &wallet->lock);
    }
    to_add -> amount += delta;
    pthread_mutex_unlock(&wallet->lock);
  } else {
      for (wallet_resource *curr=wallet->head; curr!=NULL; curr=curr->next) {
        if (!strcmp(curr->resource_name, resource)) {
          curr -> amount = curr->amount + delta;
          pthread_cond_signal(&wallet->cond);
          pthread_mutex_unlock(&wallet->lock);
          return;
        }
      }
      wallet_resource* to_add = malloc(255);
      to_add -> next = wallet -> head;
      wallet -> head = to_add;
      wallet -> head -> resource_name = resource;
      wallet -> head -> amount = delta;
      pthread_cond_signal(&wallet->cond);
      pthread_mutex_unlock(&wallet->lock);
  }
  return;
}

/**
 * Destroys a wallet, freeing all associated memory.
 */
void wallet_destroy(wallet_t *wallet) {
  // Implement this
  wallet_resource* curr = wallet->head;
  while(curr != NULL) {
    wallet_resource* next = curr->next;
    free(curr);
    curr = next;
  }
}
