/** ******************************************************************************
 *  (c) 2026 Zondax AG
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ******************************************************************************* */

import Zemu from '@zondax/zemu'
import { PolkadotGenericApp } from '@zondax/ledger-substrate'
import { readFileSync } from 'fs'
import { resolve } from 'path'
import { defaultOptions, DOT_SS58_PREFIX, PATH, models } from './common'

// @ts-expect-error missing typings
import ed25519 from 'ed25519-supercop'
import { blake2bFinal, blake2bInit, blake2bUpdate } from 'blakejs'

jest.setTimeout(600000)

// Asset Hub batch_all of 63 System.remark calls followed by a 1000 DOT
// Balances.transfer_keep_alive: 132 review items, the transfer starting at item 128.
const LONG_BATCH = JSON.parse(readFileSync(resolve('../tests/supported_chains/asset-hub-2005001.json'), 'utf8')).find(
  (v: { name: string }) => v.name === 'Utility_batch_all_63_remarks_then_transfer_1000_DOT',
)

describe('Long review', function () {
  test.concurrent.each(models)('items after 127 are shown before approval', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({ ...defaultOptions, model: m.name })
      const app = new PolkadotGenericApp(sim.getTransport(), 'dot')

      const blob = Buffer.from(LONG_BATCH.blob, 'hex')
      const metadata = Buffer.from(LONG_BATCH.metadata, 'hex')
      const { pubKey } = await app.getAddressEd25519(PATH, DOT_SS58_PREFIX)

      const signatureRequest = app.signWithMetadataEd25519(PATH, blob, metadata)
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot())

      // The transfer is item 128 onwards: the review must show its amount before approval.
      await sim.navigateUntilText('.', `${m.prefix.toLowerCase()}-long_review`, 'DOT 1000', true, false, 0, 120000, false)
      await sim.compareSnapshotsAndApprove('.', `${m.prefix.toLowerCase()}-long_review`)

      const signatureResponse = await signatureRequest
      const context = blake2bInit(32)
      blake2bUpdate(context, blob)
      const prehash = Buffer.from(blake2bFinal(context))
      expect(ed25519.verify(signatureResponse.signature.subarray(1), prehash, pubKey)).toEqual(true)
    } finally {
      await sim.close()
    }
  })
})
