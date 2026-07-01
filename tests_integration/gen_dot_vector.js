const { ApiPromise, WsProvider } = require('@polkadot/api')
const { u8aToHex, hexToU8a } = require('@polkadot/util')

const SHORTENER = 'https://polkadot-metadata-shortener.api.live.ledger.com'
// Well-known dev addresses (Alice / Bob) — content only matters for display/signature, tx is never submitted
const ALICE = '15oF4uVJwmo4TdGW7VfQxNLavjCXviqxT9S1MgbjMNHr6Sp5'
const BOB = '14Gjs1TD93gnwEBfDMHoCgsuf1s2TVKUP6Z1qKmAZnZ8cW5q'

async function shortenerHash() {
  const r = await fetch(`${SHORTENER}/node/metadata/hash`, {
    method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ id: 'dot' }),
  })
  if (!r.ok) throw new Error(`metadata/hash ${r.status}`)
  return (await r.json()).metadataHash
}

;(async () => {
  const api = await ApiPromise.create({ provider: new WsProvider('wss://rpc.polkadot.io', 3000), noInitWarn: true })
  const rt = api.runtimeVersion
  const genesis = api.genesisHash.toHex()
  const metadataHash = await shortenerHash()

  const method = api.tx.balances.forceTransfer(ALICE, BOB, 10_000_000_000n).method.toHex() // 1 DOT

  const payload = api.registry.createType('ExtrinsicPayload', {
    method,
    era: '0x00',            // immortal
    nonce: 0,
    tip: 0,
    mode: 1,                // CheckMetadataHash enabled
    metadataHash: hexToU8a('01' + metadataHash),
    specVersion: rt.specVersion.toNumber(),
    transactionVersion: rt.transactionVersion.toNumber(),
    genesisHash: genesis,
    blockHash: genesis,     // immortal -> genesis
  }, { version: 4 })

  const blobHex = u8aToHex(payload.toU8a(true)).replace(/^0x/, '')
  const tail = '01' + metadataHash
  const ok = blobHex.toLowerCase().endsWith(tail.toLowerCase())

  console.log('specVersion       :', rt.specVersion.toNumber())
  console.log('transactionVersion:', rt.transactionVersion.toNumber())
  console.log('genesis           :', genesis)
  console.log('metadataHash      :', metadataHash)
  console.log('blob ends with 01+hash?', ok)
  const template = ok ? blobHex.slice(0, blobHex.length - 64) + '<rootHash>' : '(TAIL MISMATCH) ' + blobHex
  console.log('BLOB_TEMPLATE_START')
  console.log(template)
  console.log('BLOB_TEMPLATE_END')
  console.log('ROOTHASH:', metadataHash)
  await api.disconnect()
  process.exit(0)
})().catch(e => { console.log('ERROR:', e.message); process.exit(1) })
